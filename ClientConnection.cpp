/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientConnection.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgoh <cgoh@student.42singapore.sg>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/05 17:00:54 by athonda           #+#    #+#             */
/*   Updated: 2025/08/11 21:36:41 by cgoh             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ClientConnection.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include <cstddef>
#include <cstring>
#include <ios>
#include <netdb.h>
#include <stdexcept>
#include <string>
#include <sstream>
#include <iostream>
#include <fcntl.h>
#include <cerrno>
#include <unistd.h>

static const char* const	GET = "GET";
static const char* const	POST = "POST";
static const char* const	DELETE = "DELETE";

ClientConnection::ClientConnection()
{}

ClientConnection::ClientConnection(int server_fd, Server *srv):
	addr_len(sizeof(client_addr)),
	fd(accept(server_fd, reinterpret_cast<struct sockaddr*>(&client_addr), &addr_len)),
	server(srv)
{
	if (fd < 0)
	{
		throw std::invalid_argument(strerror(errno));
	}
	// setting the new client socket as previous
	// change client socket to non-blocking mode
	if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1)
	{
		throw std::runtime_error(strerror(errno));
	}
}

ClientConnection::ClientConnection(ClientConnection const &other):
	fd(dup(other.fd)),
	server(new Server(*other.server)),
	buffer(other.buffer),
	request(other.request)
{}

ClientConnection	&ClientConnection::operator=(ClientConnection const &other)
{
	if (this != &other)
	{
		this->fd = dup(other.fd);
		this->buffer = other.buffer;
		this->request = other.request;
		delete this->server;
		this->server = new Server(*other.server);
	}
	return (*this);
}

ClientConnection::~ClientConnection()
{
	close(fd);
}

int	ClientConnection::getFd() const
{
	return (fd);
}

const HttpRequest	&ClientConnection::getRequest() const
{
	return (request);
}

const std::string	&ClientConnection::getBuffer() const
{
	return (buffer);
}

const HttpResponse	&ClientConnection::getResponse() const
{
	return (response);
}

const std::string	&ClientConnection::getResponseBuffer() const
{
	return (res_buffer);
}

std::string ClientConnection::getHost() const
{
	return std::string(host);
}

Server	*ClientConnection::getServer() const
{
	return (server);
}

void	ClientConnection::appendToBuffer(char const *data, size_t size)
{
	buffer.append(data, size);
}

bool	ClientConnection::parseRequest()
{
	if (request.is_parse_complete)
		return (true);

	size_t	pos	= buffer.find("\r\n\r\n");
	if (pos == std::string::npos)
		return (false);

	// start all of header part(until new line)
	std::stringstream	ss(buffer.substr(0, pos));
	std::string			line;

	// first line
	std::getline(ss, line);
	if (line.empty())
		return false;

	size_t	end_pos = line.find('\r');
	if (end_pos != std::string::npos)
		line.erase(end_pos);

	std::stringstream	ss_requestline(line);
	ss_requestline >> request.method >> request.uri >> request.version;
	request.is_header_parse = true;

	// header main part
	while (std::getline(ss, line) && line != "\r")
	{
		size_t	colon_pos = line.find(':');
		if (colon_pos == std::string::npos)
			return (false);

		std::string	key = line.substr(0, colon_pos);
		std::string value = line.substr(colon_pos + 2);

		size_t	epos = line.find('\r');
		if (epos != std::string::npos)
			line.erase(epos);

		request.headers[key] = value;
	}

	buffer.erase(0, pos + 4);
	// body part
	std::map<std::string, std::string>::iterator it = request.headers.find("Content-Length");
	if (it == request.headers.end())
	{
		request.is_parse_complete = true;
		return (true);
	}
	else
	{
		std::stringstream	ss_content_length(it->second);
		size_t	content_length;
		ss_content_length >> content_length;
		if (ss_content_length.fail())
			return (false);

		if (buffer.size() >= content_length)
		{
			request.body = buffer.substr(0, content_length);
			request.is_parse_complete = true;
			return true;
		}
		else
			return (false);
	}
}

std::string	ClientConnection::readFileContent(const std::string &path) const
{
	std::ifstream file(path.c_str());
	if (!file)
		return "<h1>Could not open " + path + "</h1>";
	std::stringstream ss;
	ss << file.rdbuf();
	return ss.str();
}

void	ClientConnection::sendErrorResponse(int status_code,
					const std::string &status_text,
					const std::string &error_file,
					const std::vector<std::string> &allow_methods)
{
	// Load HTML content from disk
	std::string content = readFileContent(error_file);
	
	// Set HTTP status
	response.status_code = status_code;
	response.status_message = status_text;
	
	// Body and content type
	response.setBody(content, "text/html");
	
	// Optionally add Allow header (for METHOD_NOT_ALLOWED Method Not Allowed)
	if (!allow_methods.empty())
	{
		std::string allow;
		std::vector<std::string>::const_iterator it = allow_methods.begin();
		while (it != allow_methods.end())
		{
			allow += *it;
			++it;
			if (it != allow_methods.end())
				allow += ", ";
		}
		response.addHeader("Allow", allow);
	}
	// Finalize response into the buffer that will be written to the socket
	res_buffer = response.makeString();
}


void	ClientConnection::makeResponse()
{
	try
	{
		const Location&	loc = server->getLocation(request.uri);
		if (!loc.isMethod(request.method))
		{
			sendErrorResponse(METHOD_NOT_ALLOWED, "Method not allowed", 
				server->getErrorPage(METHOD_NOT_ALLOWED),
					std::vector<std::string>());
			return ;
		}
		if (request.method == POST)
		{
			size_t	filename_start_pos = request.body.find("filename=\"");
			size_t	filename_end_pos = request.body.find("\"", filename_start_pos + 10);
			if (filename_start_pos != std::string::npos && filename_end_pos != std::string::npos)
			{
				std::string	filename = request.body.substr(filename_start_pos + 10,
					filename_end_pos - filename_start_pos - 10);
				std::ofstream	file(("tmp/" + filename).c_str(), std::ios::binary);
				if (!file)
				{
					sendErrorResponse(INTERNAL_SERVER_ERROR, "Internal Server Error",
						server->getErrorPage(INTERNAL_SERVER_ERROR),
						std::vector<std::string>());
					return;
				}
				size_t	body_start_pos = request.body.find("\r\n\r\n");
				size_t	body_end_pos = request.body.find("\r\n", body_start_pos + 4);
				file << request.body.substr(body_start_pos + 4, body_end_pos - body_start_pos - 4);
			}
			else
			{
				sendErrorResponse(BAD_REQUEST, "Bad Request",
					server->getErrorPage(BAD_REQUEST),
					std::vector<std::string>());
				return;
			}
		}
		else if (request.method == DELETE)
		{
			size_t	final_slash_pos = request.uri.rfind("/");
			std::string filename = request.uri.substr(final_slash_pos + 1);

			if (unlink(("tmp/" + filename).c_str()) == -1)
			{
				int code = errno == ENOENT ? RESOURCE_NOT_FOUND : INTERNAL_SERVER_ERROR;
				sendErrorResponse(code, "Internal Server Error",
					server->getErrorPage(code),
					std::vector<std::string>());
				return;
			}
		}
		std::string	filepath = loc.getAlias() + loc.getIndex();

		std::cout << "file path is " << filepath << std::endl;

		std::ifstream	file(filepath.c_str(), std::ios::binary);
		if (!file)
		{
			sendErrorResponse(RESOURCE_NOT_FOUND, "Not Found",
				server->getErrorPage(RESOURCE_NOT_FOUND),
				std::vector<std::string>());
			return ;
		}

		std::stringstream	ss;
		ss << file.rdbuf();
		response.status_code = OK;
		response.status_message = "OK";
		response.setBody(ss.str(), "text/html");
		res_buffer = response.makeString();
	}
	catch (const std::runtime_error &e)
	{
		std::cerr << e.what() << "\n";
		sendErrorResponse(RESOURCE_NOT_FOUND, "Not Found", 
			server->getErrorPage(RESOURCE_NOT_FOUND),
				std::vector<std::string>());
		return ;
	}
}

bool	ClientConnection::sendResponse()
{
	if (res_buffer.empty())
		return (true);

	ssize_t	n = write(getFd(), res_buffer.c_str(), res_buffer.size());
	if (n == -1)
	{
		return false;
	}
	res_buffer.erase(0, static_cast<size_t>(n));
	return (res_buffer.empty());
}

void	ClientConnection::retrieveHost()
{
	if (getnameinfo(reinterpret_cast<struct sockaddr*>(&client_addr),
	addr_len,
	host,
	sizeof(host),
	NULL,
	0,
	NI_NUMERICHOST) != 0)
	{
		throw std::runtime_error(strerror(errno));
	}
}
