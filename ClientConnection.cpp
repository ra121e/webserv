/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientConnection.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgoh <cgoh@student.42singapore.sg>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/05 17:00:54 by athonda           #+#    #+#             */
/*   Updated: 2025/09/02 19:57:48 by cgoh             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ClientConnection.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <ios>
#include <netdb.h>
#include <stdexcept>
#include <string>
#include <sstream>
#include <iostream>
#include <fcntl.h>
#include <cerrno>
#include <sys/socket.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include "CGI.hpp"
#include "Epoll.hpp"

static const char* const	GET = "GET";
static const char* const	POST = "POST";
static const char* const	DELETE = "DELETE";

ClientConnection::ClientConnection(int server_fd, Server *srv, time_t _expiry):
	BaseExpiration(_expiry),
	addr_len(sizeof(client_addr)),
	client_addr(),
	server_addr_len(sizeof(server_addr)),
	server_addr(),
	server(srv),
	server_error(false),
	timed_out(false)
{
	try
	{
		setFd(accept4(server_fd, reinterpret_cast<struct sockaddr*>(&client_addr), &addr_len, SOCK_NONBLOCK | SOCK_CLOEXEC));
	}
	catch (const std::invalid_argument&)
	{
		throw std::runtime_error("accept4: " + std::string(strerror(errno)));
	}
	if (getsockname(server_fd, reinterpret_cast<struct sockaddr*>(&server_addr), &server_addr_len) == -1)
	{
		throw std::runtime_error("getsockname: " + std::string(strerror(errno)));
	}
	retrieveHostPort(server_host, server_port, reinterpret_cast<struct sockaddr*>(&server_addr), server_addr_len);
	retrieveHostPort(host, port, reinterpret_cast<struct sockaddr*>(&client_addr), addr_len);
}

ClientConnection::ClientConnection(ClientConnection const &other):
	BaseFile(other),
	BaseExpiration(other),
	addr_len(other.addr_len),
	client_addr(other.client_addr),
	server_addr_len(other.server_addr_len),
	server_addr(other.server_addr),
	server(new Server(*other.server)),
	buffer(other.buffer),
	request(other.request),
	server_error(other.server_error),
	timed_out(other.timed_out)
{}

ClientConnection	&ClientConnection::operator=(ClientConnection const &other)
{
	if (this != &other)
	{
		addr_len = other.addr_len;
		client_addr = other.client_addr;
		BaseFile::operator=(other);
		BaseExpiration::operator=(other);
		this->buffer = other.buffer;
		this->request = other.request;
		delete this->server;
		this->server = new Server(*other.server);
		this->server_error = other.server_error;
		this->timed_out = other.timed_out;
	}
	return (*this);
}

ClientConnection::~ClientConnection()
{
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

const std::string& ClientConnection::getHost() const
{
	return host;
}

Server	*ClientConnection::getServer() const
{
	return (server);
}

void	ClientConnection::setServerError(bool error)
{
	server_error = error;
}

void	ClientConnection::setTimedOut(bool timeout)
{
	timed_out = timeout;
}

void	ClientConnection::appendToBuffer(char const *data, size_t size)
{
	buffer.append(data, size);
}

void	ClientConnection::appendToResBuffer(char const *data, size_t size)
{
	res_buffer.append(data, size);
}

bool	ClientConnection::parseRequest()
{
	if (!request.is_header_parse)
	{
		if (buffer.size() > MAX_HEADER_SIZE)
		{
			request.is_bad = true;
			return true; // Bad request, too large header
		}
		request.header_end_pos	= buffer.find("\r\n\r\n");
		if (request.header_end_pos == std::string::npos)
		{
			return false; // Not enough data to parse the request
		}

		// start all of header part(until new line)
		std::string	header_string = buffer.substr(0, request.header_end_pos);
		std::stringstream	ss(header_string);

		// first line
		ss >> request.method >> request.uri >> request.version;
		if (request.uri.rfind('.') != std::string::npos)
		{
			request.extension = request.uri.substr(request.uri.rfind('.'));
		}

		// header main part
		size_t	request_line_end = header_string.find("\r\n");
		header_string = header_string.substr(request_line_end + 2);
		for (size_t pos = header_string.find("\r\n"); pos != std::string::npos; pos = header_string.find("\r\n"))
		{
			const std::string& header_line = header_string.substr(0, pos);
			size_t	colon_pos = header_line.find(": ");
			if (colon_pos != std::string::npos)
			{
				const std::string&	key = header_line.substr(0, colon_pos);
				const std::string&	value = header_line.substr(colon_pos + 2); // skip ": "
				request.headers[key] = value;
			}
			header_string.erase(0, pos + 2); // skip "\r\n"
		}
		request.is_header_parse = true;
	}

	if (!request.waiting_for_body)
	{
		if (request.method == GET || request.method == DELETE)
		{
			return true;
		}

		// body part
		buffer.erase(0, request.header_end_pos + 4); // Remove header part from buffer
		std::map<std::string, std::string>::iterator it = request.headers.find("Content-Length");
		if (it == request.headers.end())
		{
			request.content_length_missing = true;
			return true; // Bad request, no Content-Length header
		}
		std::stringstream	ss_content_length(it->second);
		ss_content_length >> request.content_length;
		if (request.content_length > getServer()->getClientMaxBodySize())
		{
			request.body_too_large = true;
			return true;
		}
		request.waiting_for_body = true;
	}

	if (buffer.size() < request.content_length)
	{
		return false; // Not enough data to read the body
	}
	request.body = buffer;
	return true;
}

std::string	ClientConnection::readFileContent(const std::string &path)
{
	std::ifstream file(path.c_str());
	if (!file)
		return "<h1>Could not open " + path + "</h1>";
	std::stringstream ss;
	ss << file.rdbuf();
	return ss.str();
}

void	ClientConnection::sendErrorResponse(StatusCode status_code,
					const std::string &status_text,
					const std::string &error_file,
					const std::vector<std::string> &allow_methods)
{
	// Load HTML content from disk
	const std::string& content = readFileContent(error_file);

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

std::string	ClientConnection::makeIndexof(std::string const &path_dir, std::string const &uri)
{
	std::stringstream	html;
	html << "<html><body><h1>Index of " << uri << "</h1>";

	DIR	*dir = opendir(path_dir.c_str());
	if (dir)
	{
		struct dirent	*ent;
		while ((ent = readdir(dir)) != NULL)
		{
//			html << "<a href=\"" << uri << ent->d_name << "\">" << ent->d_name << "</a><p>";
			html << ent->d_name << "<p>";
		}
		closedir(dir);
	}
	html << "</body></html>";
	return (html.str());
}

void	ClientConnection::makeResponse(Epoll& epoll)
{
	try
	{
		if (request.is_bad)
		{
			sendErrorResponse(BAD_REQUEST, "Bad Request",
				server->getErrorPage(BAD_REQUEST),
				std::vector<std::string>());
			return;
		}
		if (server_error)
		{
			sendErrorResponse(INTERNAL_SERVER_ERROR, "Internal Server Error",
				server->getErrorPage(INTERNAL_SERVER_ERROR),
				std::vector<std::string>());
			return;
		}
		if (timed_out)
		{
			sendErrorResponse(REQUEST_TIMEOUT, "Request Timeout",
				server->getErrorPage(REQUEST_TIMEOUT),
				std::vector<std::string>());
			return;
		}
		if (request.body_too_large)
		{
			sendErrorResponse(PAYLOAD_TOO_LARGE, "Payload Too Large",
				server->getErrorPage(PAYLOAD_TOO_LARGE),
				std::vector<std::string>());
			return;
		}
		const Location&	loc = server->getLocation(request.uri, request.extension, request);
		if (loc.getIsRedirect())
		{
			response.addHeader("Location", loc.getRedirectTarget());
			sendErrorResponse(static_cast<StatusCode>(loc.getRedirectCode()),
			"Redirecting",
			server->getErrorPage(loc.getRedirectCode()),
			std::vector<std::string>());
			return ;
		}
		if (!loc.isMethod(request.method))
		{
			sendErrorResponse(METHOD_NOT_ALLOWED, "Method not allowed",
				server->getErrorPage(METHOD_NOT_ALLOWED),
					loc.getMethods());
			return ;
		}
		if (request.forward_to_cgi)
		{
			run_cgi_script(request.uri, epoll);
			return;
		}
		// if (is_cgi_script(request.uri))
		// {
		// 	std::cout << "getAlias : " << loc.getAlias() << std::endl;
		// 	std::cout << "getUri : " << request.uri << std::endl;
		// 	std::string uri_path = request.uri;
		// 	if (uri_path.rfind("/cgi-bin/", 0) == 0)
		// 		uri_path = uri_path.substr(9);
		// 	std::cout << "uriPath : " << uri_path << std::endl;
		// 	std::string script_path = loc.getAlias() + uri_path;
		// 	std::cout << "script_path" << script_path << std::endl;
		// 	int	ret = run_cgi_script(*this, script_path, *(getServer()->getNetworks()[0]));
		// 	if (ret != 0)
		// 		sendErrorResponse(INTERNAL_SERVER_ERROR, "CGI Excecution failed",
		// 			server->getErrorPage(INTERNAL_SERVER_ERROR),
		// 			std::vector<std::string>());
		// 	return ;
		// }
		if (request.method == POST)
		{
			if (request.content_length_missing)
			{
				sendErrorResponse(CONTENT_LENGTH_MISSING, "Content-Length Missing",
					server->getErrorPage(CONTENT_LENGTH_MISSING),
					std::vector<std::string>());
				return;
			}
			const std::string& FILENAME_FIELD = "filename=\"";
			size_t	filename_start_pos = request.body.find(FILENAME_FIELD);
			size_t	filename_end_pos = request.body.find('\"', filename_start_pos + FILENAME_FIELD.size());
			if (filename_start_pos != std::string::npos && filename_end_pos != std::string::npos)
			{
				const std::string& filename = request.body.substr(filename_start_pos + FILENAME_FIELD.size(),
					filename_end_pos - filename_start_pos - FILENAME_FIELD.size());
				const std::string&	filepath = "tmp/" + filename;
				std::ofstream	file(filepath.c_str(), std::ios::binary);
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
			size_t	final_slash_pos = request.uri.rfind('/');
			const std::string& filename = request.uri.substr(final_slash_pos + 1);

			if (unlink(("tmp/" + filename).c_str()) == -1)
			{
				StatusCode code = errno == ENOENT ? RESOURCE_NOT_FOUND : INTERNAL_SERVER_ERROR;
				sendErrorResponse(code, "Internal Server Error",
					server->getErrorPage(code),
					std::vector<std::string>());
				return;
			}
		}
		const std::string& filepath = loc.getAlias() + loc.getIndex();

		struct stat file_stat = {};
		if (stat(filepath.c_str(), &file_stat) == 0)
		{
			if (S_ISREG(file_stat.st_mode))
			{
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
				const std::string& extension = getFileExtension(filepath);
				std::string content_type = "application/octet-stream";
				if (extension == "html")
				{
					content_type = "text/html";
				}
				else if (extension == "css")
				{
					content_type = "text/css";
				}
				else if (extension == "js")
				{
					content_type = "application/javascript";
				}
				response.setBody(ss.str(), content_type);
				res_buffer = response.makeString();
			}
			else if (S_ISDIR(file_stat.st_mode))
			{
				if (loc.isAutoindexOn())
				{
					response.status_code = OK;
					response.status_message = "OK";
					response.setBody(makeIndexof(filepath, request.uri), "text/html");
					res_buffer = response.makeString();
				}
				else
				{
					sendErrorResponse(RESOURCE_NOT_FOUND, "Not Found",
						server->getErrorPage(RESOURCE_NOT_FOUND),
						std::vector<std::string>());
					return ;
				}
			}
		}
	}
	catch (const std::runtime_error&)
	{
		sendErrorResponse(RESOURCE_NOT_FOUND, "Not Found",
			server->getErrorPage(RESOURCE_NOT_FOUND),
				std::vector<std::string>());
		return ;
	}
}

bool	ClientConnection::sendResponse()
{
	if (res_buffer.empty())
	{
		return true;
	}

	ssize_t	bytes_written = write(getFd(), res_buffer.c_str(), res_buffer.size());
	if (bytes_written == -1)
	{
		return false;
	}
	res_buffer.erase(0, static_cast<size_t>(bytes_written));
	return (res_buffer.empty());
}

void	ClientConnection::retrieveHostPort(std::string& _host, std::string& _port, struct sockaddr* addr, socklen_t _addr_len)
{
	char host_buffer[NI_MAXHOST];
	char port_buffer[NI_MAXSERV];

	int gni_value = getnameinfo(addr,
		_addr_len,
		host_buffer,
		sizeof(host_buffer),
		port_buffer,
		sizeof(port_buffer),
		NI_NUMERICHOST | NI_NUMERICSERV);
	if (gni_value != 0)
	{
		throw std::runtime_error("getnameinfo: " + std::string(gai_strerror(gni_value)));
	}
	_host = host_buffer;
	_port = port_buffer;
}

std::string ClientConnection::getFileExtension(const std::string& filepath)
{
	size_t pos = filepath.rfind('.');
	if (pos == std::string::npos || pos == filepath.size() - 1)
	{
		return "";
	}
	return filepath.substr(pos + 1);
}

void	ClientConnection::run_cgi_script(const std::string& script_path, Epoll& epoll)
{ 
	CGI* cgi = new CGI(this); // pass the current ClientConnection instance to CGI constructor //

	cgi->add_env("REQUEST_METHOD", request.method);
	cgi->add_env("QUERY_STRING", getQueryString(request.uri));
	cgi->add_env("CONTENT_LENGTH", getContentLength(request.headers));
	cgi->add_env("CONTENT_TYPE", getContentType(request.headers));
	cgi->add_env("SCRIPT_NAME", getScriptName(request.uri));
	cgi->add_env("SERVER_NAME", server_host);
	cgi->add_env("SERVER_PORT", server_port);
	cgi->add_env("SERVER_PROTOCOL", request.version);
	cgi->add_env("REMOTE_ADDR", host);
	cgi->add_env("HTTP_USER_AGENT", getUserAgent(request.headers));
	cgi->add_env("HTTP_COOKIE", getCookie(request.headers));
	cgi->add_env("HTTP_REFERER", getReferer(request.headers));
	cgi->add_env("GATEWAY_INTERFACE", "CGI/1.1");
	cgi->add_env("PATH_INFO", getPathInfo(script_path));

	cgi->convert_env_map_to_envp(); // function to copy all the env_map variables into a environment so that i can run execve //

	pid_t	pid = fork(); // forking to let the child inherit all the env_variables // 
	switch (pid) {
		case -1:
			throw std::runtime_error("fork: " + std::string(strerror(errno)));
		case 0:
			cgi->execute_cgi();
			break;
		default:
			cgi->close_pipes();
			epoll.addPipeFds(cgi);
			if (request.method == "POST")
			{
				epoll.modifyEpoll(cgi->get_server_write_fd(), EPOLLOUT, EPOLL_CTL_ADD);
			}
	}
}
		
		// Set server_read_cgi_write_pipe[0] to non-blocking to properly use edge-triggered epoll
		// int flags = fcntl(server_read_cgi_write_pipe[0], F_GETFL, 0); // file access mode and status flags//
		// fcntl(server_read_cgi_write_pipe[0], F_SETFL, flags | O_NONBLOCK); // set it to nonblocking //
		
		// std::string cgi_output;
		// const int TIMEOUT_MS = 8000; // 3 seconds timeout
		// bool done = false;
		// bool timeout_occurred = false;
		// while (!done)
		// {
		// 	struct epoll_event events[1];
		// 	int nfds = epoll_wait(epfd, events, 1, TIMEOUT_MS);
			
		// 	if (nfds == 0) // timeout
		// 	{
		// 		std::cerr << "Timeout waiting for CGI output. Killing child process." << std::endl;
		// 		kill(pid, SIGKILL);
		// 		timeout_occurred = true;
		// 		done = true;
		// 		break;
		// 	}
		// 	else if (nfds == -1) // error
		// 	{
		// 		if (errno == EINTR) // interrupted by signal, try again
		// 			continue; 
		// 		perror("epoll_wait");
		// 		std::cerr << "Error in epoll_wait. Killing child process." << std::endl;
		// 		kill(pid, SIGKILL);
		// 		timeout_occurred = true;
		// 		done = true;
		// 		break;
		// 	}
		// 	else
		// 	{
		// 		if (events[0].data.fd == server_read_cgi_write_pipe[0])
		// 		{
		// 			while (true)
		// 			{
		// 				char buf[BUFFER_SIZE];
		// 				ssize_t bytes_read = read(server_read_cgi_write_pipe[0], buf, sizeof(buf));
		// 				if (bytes_read == -1)
		// 				{
		// 					if (errno == EAGAIN)
		// 					{
		// 						// No more data for now
		// 						break;
		// 					}
		// 					else
		// 					{
		// 						perror("read from CGI stdout");
		// 						done = true;
		// 						break;
		// 					}
		// 				}
		// 				else if (bytes_read == 0)
		// 				{
		// 					// EOF - pipe closed by child
		// 					done = true;
		// 					break;
		// 				}
		// 				else
		// 				{
		// 					cgi_output.append(buf, static_cast<size_t>(bytes_read));
		// 				}
		// 			}
		// 		}
		// 	}
		// }
		// close(server_read_cgi_write_pipe[0]); // close server_read_cgi_write_pipe once data is read //
		// close(epfd); // close epoll fd as well //
		
		// if (!timeout_occurred) // if timeout has not occured, we monitor the status flags // 
		// {
		// 	int status;
		// 	pid_t result = waitpid(pid, &status, WNOHANG);
		// 	if (result == 0)
		// 	{
		// 	}
		// 	else if (result == pid)
		// 	{
		// 		if (WIFEXITED(status))
		// 		{
		// 			int exit_code = WEXITSTATUS(status);
		// 			std::cout << "CGI script exited normally with code "
		// 				<< exit_code << std::endl;
		// 		}
		// 		else if (WIFSIGNALED(status))
		// 		{
		// 			int term_sig = WTERMSIG(status);
		// 			std::cout << "CGI script was terminated by signal "
		// 				<< term_sig << std::endl;
		// 		}
		// 		// Mark that you are done reading from CGI output because the child terminated.
		// 		done = true;
		// 	}		
		// }
		// free_envp(cgi_envp);
		// // Use cgi_output as the CGI script response content
		// std::cout << "Parent received:\n" << cgi_output << std::endl;
