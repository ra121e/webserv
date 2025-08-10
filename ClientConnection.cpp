/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientConnection.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgoh <cgoh@student.42singapore.sg>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/05 17:00:54 by athonda           #+#    #+#             */
/*   Updated: 2025/08/10 22:12:15 by cgoh             ###   ########.fr       */
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
			size_t	filename_start_pos = request.body.find("filename=\"");
			size_t	filename_end_pos = request.body.find("\"", filename_start_pos + 10);
			if (filename_start_pos != std::string::npos && filename_end_pos != std::string::npos)
			{
				std::string	filename = request.body.substr(filename_start_pos + 10, filename_end_pos - filename_start_pos - 10);
				std::ofstream	file(("tmp/" + filename).c_str(), std::ios::binary);
				if (!file)
				{
					throw std::ios::failure("Error opening file: " + filename);
				}
				size_t	body_start_pos = request.body.find("\r\n\r\n");
				size_t	body_end_pos = request.body.find("\r\n", body_start_pos + 4);
				file << request.body.substr(body_start_pos + 4, body_end_pos - body_start_pos - 4);
				file.close();
				return (true);
			}
			else
			{
				throw std::runtime_error("Filename not found in request body");
			}
		}
		else
			return (false);
	}
}

void	ClientConnection::makeResponse()
{
//	Location	*loc = server->getLocation(request.uri);
	std::pair<Location*, size_t>	match = server->getLocation(request.uri);
	Location *loc = match.first;
	std::cout << "match location: " << loc->getIndex() << std::endl;
	if (!loc)
	{
		std::cout << "here in !loc: " << loc->getIndex() << std::endl;
		response.status_code = 404;
		response.status_message = "Not Found";
		response.setBody("<h1>404 Not Found</h1>", "text/html");
		res_buffer = response.makeString();
		return ;
	}
	if (!loc->isMethod(request.method))
	{
		response.status_code = 405;
		response.status_message = "Method Not Allowed";
		response.setBody("<h1>405 Method Not Allowed</h1>", "text/html");
		res_buffer = response.makeString();
		return ;
	}

	std::string	filepath;
	if (!loc->getAlias().empty())
		filepath = loc->getAlias() + request.uri.substr(match.second);
	else
		filepath = request.uri;

	std::cout << "file path is " << filepath << std::endl;

	std::ifstream	file(filepath.c_str(), std::ios::binary);
	if (!file)
	{
		std::cout << "here in !file: " << loc->getIndex() << std::endl;
		response.status_code = 404;
		response.status_message = "Not Found";
		response.setBody("<h1>404 Not Found</h1>", "text/html");
		res_buffer = response.makeString();
		return ;
	}

	std::stringstream	ss;
	ss << file.rdbuf();
	response.status_code = 200;
	response.status_message = "OK";
	response.setBody(ss.str(), "text/html");
	res_buffer = response.makeString();

//	if (request.method == "GET" && request.uri == "/")
//	{
//		response.setBody("<h1>Hello, 42World!</h1>", "text/html");
//	}
//	else
//	{
//		response.status_code = 404;
//		response.status_message = "Not Found";
//		response.setBody("<h1>404 Not Found</h1>", "text/html");
//	}
}

bool	ClientConnection::sendResponse()
{
	if (res_buffer.empty())
		return (true);

	ssize_t	n = write(getFd(), res_buffer.c_str(), res_buffer.size());
	if (n == -1)
	{
		if (errno == EAGAIN)
			return (false);
		return (false);
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
