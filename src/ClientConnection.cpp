/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientConnection.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgoh <cgoh@student.42singapore.sg>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/05 17:00:54 by athonda           #+#    #+#             */
/*   Updated: 2025/09/15 21:56:26 by cgoh             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/ClientConnection.hpp"
#include "../include/HttpRequest.hpp"
#include "../include/HttpResponse.hpp"
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <ios>
#include <map>
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
#include "../include/CGI.hpp"
#include "../include/Epoll.hpp"
#include <iomanip>
#include <utility>
#include <vector>
#include "../include/Directory.hpp"

static const char GET[] = "GET";
static const char POST[] = "POST";
static const char DELETE[] = "DELETE";
static const char DOUBLE_CRLF[] = "\r\n\r\n";
static const char CRLF[] = "\r\n";

ClientConnection::ClientConnection(int server_fd, const SharedPointer<Server>& srv, time_t _expiry):
	BaseExpiration(_expiry),
	addr_len(sizeof(client_addr)),
	client_addr(),
	server_addr_len(sizeof(server_addr)),
	server_addr(),
	server(srv),
	server_error(false),
	timed_out(false),
	cgi_timed_out(false)
{
	try
	{
		setFd(accept4(server_fd, reinterpret_cast<struct sockaddr*>(&client_addr),
		&addr_len, SOCK_NONBLOCK | SOCK_CLOEXEC));
	}
	catch (const std::invalid_argument&)
	{
		throw std::runtime_error("accept4: " + std::string(strerror(errno)));
	}
	if (getsockname(server_fd, reinterpret_cast<struct sockaddr*>(&server_addr),
	&server_addr_len) == -1)
	{
		throw std::runtime_error("getsockname: " + std::string(strerror(errno)));
	}
	retrieveHostPort(server_host, server_port,
		reinterpret_cast<struct sockaddr*>(&server_addr), server_addr_len);
	retrieveHostPort(host, port,
		reinterpret_cast<struct sockaddr*>(&client_addr), addr_len);
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

const SharedPointer<Server>&	ClientConnection::getServer() const
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

void	ClientConnection::setCgiTimedOut(bool timeout)
{
	cgi_timed_out = timeout;
}

void	ClientConnection::appendToBuffer(char const *data, size_t size)
{
	buffer.append(data, size);
}

void	ClientConnection::appendToResBuffer(char const *data, size_t size)
{
	res_buffer.append(data, size);
}

bool	ClientConnection::parseRequestHeader()
{	
	if (buffer.size() > MAX_HEADER_SIZE)
	{
		request.is_bad = true;
		return true; // Bad request, too large header
	}
	request.header_end_pos	= buffer.find(DOUBLE_CRLF);
	if (request.header_end_pos == std::string::npos)
	{
		return false; // Not enough data to parse the request
	}

	// start all of header part(until new line)
	std::string	header_string = buffer.substr(0, request.header_end_pos + sizeof(CRLF) - 1);
	std::stringstream	ss(header_string);

	// first line
	ss >> request.method >> request.uri >> request.version;
	if (request.uri.rfind('.') != std::string::npos)
	{
		request.extension = request.uri.substr(request.uri.rfind('.'));
	}

	// header main part
	size_t	request_line_end = header_string.find(CRLF);
	header_string = header_string.substr(request_line_end + sizeof(CRLF) - 1); // Remove request line from header_string
	for (size_t pos = header_string.find(CRLF); pos != std::string::npos; pos = header_string.find(CRLF))
	{
		const std::string& header_line = header_string.substr(0, pos);
		size_t	colon_pos = header_line.find(": ");
		if (colon_pos != std::string::npos)
		{
			const std::string&	key = header_line.substr(0, colon_pos);
			const std::string&	value = header_line.substr(colon_pos + sizeof(CRLF) - 1); // skip ": "
			request.headers[key] = value;
		}
		header_string.erase(0, pos + sizeof(CRLF) - 1); // skip "\r\n"
	}
	request.is_header_parse = true;
	return true;	
}

bool	ClientConnection::parseRequestBody()
{
	// body part
	buffer.erase(0, request.header_end_pos + sizeof(DOUBLE_CRLF) - 1); // Remove header part from buffer
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
	return true;
}

bool	ClientConnection::parseRequest()
{	
	if (!request.is_header_parse)
	{
		if (!parseRequestHeader())
			return false; // Not enough data to parse the request
	}

	if (!request.waiting_for_body)
	{
		if (request.method == GET || request.method == DELETE)
		{
			return true;
		}
		if (!parseRequestBody())
			return false;
	}

	if (buffer.size() < request.content_length)
	{
		return false; // Not enough data to read the body
	}
	request.body = buffer;
	return true;
}

std::string	ClientConnection::readFileContent(StatusCode status_code,
	const std::string& status_text, const std::string &path)
{
	std::ifstream file(path.c_str());
	std::stringstream	ss;
	if (!file)
	{
		ss << "<html><head><title>" << status_code << " " << status_text << "</title></head>";
		// Add a body with the error message
		ss << "<body><h1>" << status_code << " " << status_text << "</h1></body></html>";
		return ss.str();
	}
	ss << file.rdbuf();
	return ss.str();
}

void	ClientConnection::sendErrorResponse(StatusCode status_code,
	const std::string &status_text,const std::string& error_file)
{
	// Load HTML content from disk
	const std::string& content = readFileContent(status_code, status_text, error_file);

	// Set HTTP status
	response.status_code = status_code;
	response.status_message = status_text;

	// Body and content type
	response.setBody(content, "text/html");

	// Finalize response into the buffer that will be written to the socket
	res_buffer = response.makeString();
}

std::string	ClientConnection::makeIndexof(std::string const &path_dir, std::string const &uri)
{
	std::stringstream	html;
	html << "<html><body><h1>Index of " << uri << "</h1>";

	Directory dir(path_dir.c_str());
	struct dirent	*ent = NULL;
	while ((ent = dir.read()) != NULL)
	{
//			html << "<a href=\"" << uri << ent->d_name << "\">" << ent->d_name << "</a><p>";
		html << ent->d_name << "<p>";
	}
	html << "</body></html>";
	return (html.str());
}

void	ClientConnection::makeResponse(Epoll& epoll, const SharedPointer<ClientConnection>& client_ptr)
{
	if (request.is_bad)
	{
		sendErrorResponse(BAD_REQUEST,
			"Bad Request",
			server->getErrorPage(BAD_REQUEST));
		return;
	}
	if (server_error)
	{
		sendErrorResponse(INTERNAL_SERVER_ERROR,
			"Internal Server Error",
			server->getErrorPage(INTERNAL_SERVER_ERROR));
		return;
	}
	if (timed_out)
	{
		sendErrorResponse(REQUEST_TIMEOUT,
			"Request Timeout",
			server->getErrorPage(REQUEST_TIMEOUT));
		return;
	}
	if (cgi_timed_out)
	{
		sendErrorResponse(GATEWAY_TIMEOUT,
			"Gateway Timeout",
			server->getErrorPage(GATEWAY_TIMEOUT));
		return;
	}
	if (request.body_too_large)
	{
		sendErrorResponse(PAYLOAD_TOO_LARGE,
			"Payload Too Large",
			server->getErrorPage(PAYLOAD_TOO_LARGE));
		return;
	}

	std::map<std::string, Location>::const_iterator loc_it =
	server->getLocationIteratorMatch(request.uri, request.extension, request);

	if (loc_it == server->getLocations().end())
	{
		sendErrorResponse(RESOURCE_NOT_FOUND,
			"Resource Not Found",
			server->getErrorPage(RESOURCE_NOT_FOUND));
		return;
	}

	const Location&	loc = loc_it->second;

	if (loc.getIsRedirect())
	{
		if (request.uri == "/logout") {
			// Clear the session cookie by setting it to expire in the past
			response.addHeader("Set-Cookie",
				"session_id=; Path=/; Expires=Thu, 01 Jan 1970 00:00:00 GMT; SameSite=Lax");
			server->removeSessionId(request.getCookieValue("session_id"));
		}
		response.addHeader("Location", loc.getRedirectTarget());
		sendErrorResponse(static_cast<StatusCode>(loc.getRedirectCode()),
		"Redirecting",
		server->getErrorPage(loc.getRedirectCode()));
		return;
	}
	if (!loc.isMethod(request.method))
	{
		response.addHeader("Allow", loc.getMethodsStrRep());
		sendErrorResponse(METHOD_NOT_ALLOWED,
			"Method Not Allowed",
			server->getErrorPage(METHOD_NOT_ALLOWED));
		return;
	}
	if (request.forward_to_cgi)
	{
		run_cgi_script(epoll, client_ptr);
		return;
	}
	if (request.method == POST)
	{
		if (request.uri == "/login")
		{
			StatusCode code = handleLogin();
			if (code != OK)
			{
				sendErrorResponse(code,
					code == UNAUTHORIZED ? "Unauthorized" : "Bad Request",
					server->getErrorPage(code));
				return;
			}
		}
		else if (request.uri == "/register")
		{
			StatusCode code = handleRegistration();
			if (code != OK)
			{
				sendErrorResponse(code,
					code == CONFLICT ? "Conflict" : "Bad Request",
					server->getErrorPage(code));
				return;
			}
		}
	}
	else if (request.method == DELETE)
	{
		size_t	final_slash_pos = request.uri.rfind('/');
		const std::string& filename = request.uri.substr(final_slash_pos + 1);

		if (unlink(("tmp/" + filename).c_str()) == -1)
		{
			StatusCode	status_code = errno == ENOENT ? RESOURCE_NOT_FOUND : INTERNAL_SERVER_ERROR;
			sendErrorResponse(status_code,
				"Error deleting file",
				server->getErrorPage(status_code));
			return;
		}
	}
	if (request.uri == "/lounge")
	{
		if (!checkSessionCookie())
		{
			response.addHeader("Location", "/login");
			sendErrorResponse(SEE_OTHER,
				"Redirecting",
				server->getErrorPage(SEE_OTHER));
			return;
		}
	}
	const std::string& filepath = loc.getAlias() + loc.getIndex();

	struct stat file_stat = {};
	if (stat(filepath.c_str(), &file_stat) != 0)
	{
		switch (errno) {
			case ENOENT:
				sendErrorResponse(RESOURCE_NOT_FOUND,
					"Not Found",
					server->getErrorPage(RESOURCE_NOT_FOUND));
				break;
			case EACCES:
				sendErrorResponse(FORBIDDEN,
					"Forbidden",
					server->getErrorPage(FORBIDDEN));
				break;
			default:
				sendErrorResponse(INTERNAL_SERVER_ERROR,
				"Error retrieving file information",
				server->getErrorPage(INTERNAL_SERVER_ERROR));
				break;
		}
		return;
	}
	if (S_ISREG(file_stat.st_mode))
	{
		std::ifstream	file(filepath.c_str(), std::ios::binary);
		if (!file)
		{
			sendErrorResponse(INTERNAL_SERVER_ERROR,
				"Error opening file",
				server->getErrorPage(INTERNAL_SERVER_ERROR));
			return;
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
			try
			{
				response.setBody(makeIndexof(filepath, request.uri),
				"text/html");
			}
			catch (const std::exception&)
			{
				sendErrorResponse(INTERNAL_SERVER_ERROR,
					"Error generating directory index",
					server->getErrorPage(INTERNAL_SERVER_ERROR));
				return;
			}
			res_buffer = response.makeString();
		}
		else
		{
			sendErrorResponse(FORBIDDEN,
				"Forbidden",
				server->getErrorPage(FORBIDDEN));
		}
	}
	else
	{
		sendErrorResponse(FORBIDDEN,
			"Forbidden",
			server->getErrorPage(FORBIDDEN));
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

void	ClientConnection::run_cgi_script(Epoll& epoll, const SharedPointer<ClientConnection>& client_ptr)
{
	std::pair<std::string, std::string>	cgi_params[] = {
		std::make_pair("REQUEST_METHOD", request.method),
		std::make_pair("QUERY_STRING", request.getQueryString()),
		std::make_pair("CONTENT_LENGTH", request.getContentLength()),
		std::make_pair("CONTENT_TYPE", request.getContentType()),
		std::make_pair("SCRIPT_NAME", request.getScriptName()),
		std::make_pair("SERVER_NAME", server_host),
		std::make_pair("SERVER_PORT", server_port),
		std::make_pair("SERVER_PROTOCOL", request.version),
		std::make_pair("REMOTE_ADDR", host),
		std::make_pair("HTTP_COOKIE", request.getCookie()),
		std::make_pair("GATEWAY_INTERFACE", "CGI/1.1"),
		std::make_pair("PATH_INFO", request.getPathInfo())
	};

	SharedPointer<CGI> cgi(new CGI(client_ptr, cgi_params, sizeof(cgi_params) / sizeof(cgi_params[0])));

	epoll.addPipeFds(cgi);
	cgi->setPid(fork()); // forking to let the child inherit all the env_variables // 
	switch (cgi->getPid()) {
		case -1:
			throw std::runtime_error("fork: " + std::string(strerror(errno)));
		case 0:
			cgi->execute_cgi();
			break;
		default:
			cgi->close_pipes();
			if (request.method == "POST")
			{
				epoll.modifyEpoll(cgi->get_server_write_fd(), EPOLLOUT, EPOLL_CTL_ADD);
			}
			epoll.addCgiExpiry(cgi);
	}
}

ClientConnection::StatusCode	ClientConnection::handleLogin()
{
	const std::string	USERNAME_FIELD = "username=";
	const std::string	PASSWORD_FIELD = "&password=";
	
	// Parse username and password from request body
	std::size_t	username_pos = request.body.find(USERNAME_FIELD);
	std::size_t	password_pos = request.body.find(PASSWORD_FIELD);
	if (username_pos == std::string::npos || password_pos == std::string::npos)
	{
		return BAD_REQUEST;
	}
	std::string username = request.body.substr(username_pos + USERNAME_FIELD.size(),
	password_pos - (username_pos + USERNAME_FIELD.size()));
	std::string password = request.body.substr(password_pos + PASSWORD_FIELD.size());
	if (!server->authenticateUser(username, password))
	{
		return UNAUTHORIZED;
	}
	const std::string& session_id = generateSessionId(32); // Generate a 32-byte (64 hex characters) session ID
	server->addSessionId(session_id);
	response.addHeader("Set-Cookie", "session_id=" + session_id + "; Path=/");
	return OK;
}

ClientConnection::StatusCode	ClientConnection::handleRegistration()
{
	const std::string	USERNAME_FIELD = "username=";
	const std::string	PASSWORD_FIELD = "&password=";
	const std::string	CONFIRM_PASSWORD_FIELD = "&confirm=";
	
	// Parse username and password from request body
	std::size_t	username_pos = request.body.find(USERNAME_FIELD);
	std::size_t	password_pos = request.body.find(PASSWORD_FIELD);
	std::size_t	confirm_password_pos = request.body.find(CONFIRM_PASSWORD_FIELD);
	if (username_pos == std::string::npos
		|| password_pos == std::string::npos
		|| confirm_password_pos == std::string::npos)
	{
		return  BAD_REQUEST;
	}
	std::string username = request.body.substr(username_pos + USERNAME_FIELD.size(),
	password_pos - (username_pos + USERNAME_FIELD.size()));
	std::string password = request.body.substr(password_pos + PASSWORD_FIELD.size(),
		confirm_password_pos - (password_pos + PASSWORD_FIELD.size()));
	if (!server->addUser(username, password))
	{
		return CONFLICT;
	}
	return OK;
}

// Generate a cryptographically secure random session ID
std::string ClientConnection::generateSessionId(std::size_t length_bytes) {
	static const std::size_t MAX_LENGTH = 64; // Maximum length in bytes
    std::vector<char> buffer(MAX_LENGTH);  // up to 64 random bytes
    length_bytes = std::min(length_bytes, buffer.size());

    // Read from /dev/urandom
    std::ifstream urandom("/dev/urandom", std::ios::in | std::ios::binary);
    if (!urandom) {
        throw std::ios_base::failure("Failed to open /dev/urandom");
    }
    urandom.read(buffer.data(), static_cast<std::streamsize>(length_bytes));
    if (!urandom) {
        throw std::ios_base::failure("Failed to read from /dev/urandom");
    }

    // Convert to hex string
    std::ostringstream oss;
    for (std::size_t i = 0; i < length_bytes; ++i) {
        oss << std::hex << std::setw(2) << std::setfill('0')
		<< static_cast<unsigned int>(static_cast<unsigned char>(buffer[i]));
    }

    return oss.str();  // e.g. "9f2a7b13d4c8e6..."
}

bool	ClientConnection::checkSessionCookie()
{
	std::map<std::string, std::string>::const_iterator it = request.headers.find("Cookie");
	if (it == request.headers.end())
	{
		return false;
	}
	const std::string&	cookie_header = it->second;
	const std::string	session_prefix = "session_id=";
	size_t	session_pos = cookie_header.find(session_prefix);
	if (session_pos == std::string::npos)
	{
		return false;
	}
	std::string session_id = cookie_header.substr(session_pos + session_prefix.size());
	size_t	semicolon_pos = session_id.find(';');
	if (semicolon_pos != std::string::npos)
	{
		session_id = session_id.substr(0, semicolon_pos);
	}
	if (!server->isValidSessionId(session_id))
	{
		return false;
	}
	return true;
}
