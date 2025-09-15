/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientConnection.hpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgoh <cgoh@student.42singapore.sg>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/05 16:52:27 by athonda           #+#    #+#             */
/*   Updated: 2025/09/15 22:02:08 by cgoh             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENTCONNECTION_HPP
# define CLIENTCONNECTION_HPP
// Minimal includes to reduce coupling; full definitions pulled in .cpp
# include "HttpRequest.hpp"
# include "HttpResponse.hpp"
# include "Server.hpp"
#include <cstddef>
#include <ctime>
#include <netdb.h>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include "BaseFile.hpp"
#include "BaseExpiration.hpp"

class Epoll; // forward declaration
class CGI;   // forward declaration

class ClientConnection : public BaseFile, public BaseExpiration
{
	public:
		ClientConnection(int server_fd, const SharedPointer<Server>& srv, time_t _expiry);

		const HttpRequest	&getRequest() const;
		const std::string	&getBuffer() const;

		const HttpResponse	&getResponse() const;
		const std::string	&getResponseBuffer() const;
		const std::string&	getHost() const;
		const SharedPointer<Server>&	getServer() const;
		void	setServerError(bool error);
		void	setTimedOut(bool timeout);
		void	setCgiTimedOut(bool timeout);

		void	appendToBuffer(char const *data, size_t size);
		void	appendToResBuffer(char const *data, size_t size);
		bool	parseRequest();
		void	makeResponse(Epoll& epoll, const SharedPointer<ClientConnection>& client_ptr);
		bool	sendResponse();
		static void	retrieveHostPort(std::string& _host, std::string& _port,
			struct sockaddr* addr, socklen_t _addr_len);
		void	run_cgi_script(Epoll& epoll, const SharedPointer<ClientConnection>& client_ptr);

	private:
		socklen_t			addr_len;
		struct sockaddr_in	client_addr;
		socklen_t			server_addr_len;
		struct sockaddr_storage	server_addr;
		SharedPointer<Server>	server;
		std::string			server_host;
		std::string			server_port;
		std::string			host;
		std::string			port;
		std::string			buffer;
		std::string			res_buffer;
		HttpRequest			request;
		HttpResponse		response;
		bool				server_error;
		bool				timed_out;
		bool				cgi_timed_out;
		enum StatusCode
		{
			OK = 200,
			MOVED_PERMANENTLY = 301,
			SEE_OTHER = 303,
			BAD_REQUEST = 400,
			UNAUTHORIZED = 401,
			FORBIDDEN = 403,
			RESOURCE_NOT_FOUND = 404,
			METHOD_NOT_ALLOWED = 405,
			REQUEST_TIMEOUT = 408,
			CONFLICT = 409,
			CONTENT_LENGTH_MISSING = 411,
			PAYLOAD_TOO_LARGE = 413,
			INTERNAL_SERVER_ERROR = 500,
			GATEWAY_TIMEOUT = 504
		};
		static const std::size_t	MAX_HEADER_SIZE = 8192;

		static std::string	readFileContent(StatusCode status_code,
			const std::string& status_text,
			const std::string &path);
		void		sendErrorResponse(StatusCode status_code,
                        const std::string &status_text,
                        const std::string& error_file);
		static std::string	makeIndexof(std::string const &path_dir, std::string const &uri);
		static std::string	getFileExtension(const std::string& filepath);
		StatusCode		handleLogin();
		StatusCode		handleRegistration();
		static std::string	generateSessionId(std::size_t length);
		bool	checkSessionCookie();
		bool	parseRequestHeader();
		bool	parseRequestBody();
};


#endif
