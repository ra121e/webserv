/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientConnection.hpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgoh <cgoh@student.42singapore.sg>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/05 16:52:27 by athonda           #+#    #+#             */
/*   Updated: 2025/08/25 14:52:37 by cgoh             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENTCONNECTION_HPP
# define CLIENTCONNECTION_HPP

class ClientConnection;
# include "HttpRequest.hpp"
# include "HttpResponse.hpp"
# include "cgi_handler.hpp"
# include "Server.hpp"
#include <cstddef>
#include <ctime>
#include <netdb.h>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include "BaseFile.hpp"
#include "Epoll.hpp"

class ClientConnection : public BaseFile
{
	public:
		ClientConnection(int server_fd, Server *srv, time_t _expiry);
		ClientConnection(ClientConnection const &other);
		ClientConnection	&operator=(ClientConnection const &other);
		~ClientConnection();

		const HttpRequest	&getRequest() const;
		const std::string	&getBuffer() const;

		const HttpResponse	&getResponse() const;
		const std::string	&getResponseBuffer() const;
		const std::string& getHost() const;
		Server	*getServer() const;
		void	setServerError(bool error);
		void	setTimedOut(bool timeout);
		void	setExpiry(time_t _expiry);
		time_t	getExpiry() const;

		void	appendToBuffer(char const *data, size_t size);
		bool	parseRequest();
		void	makeResponse(Epoll& epoll);
		bool	sendResponse();
		void	retrieveHostPort(std::string& _host, std::string& _port, struct sockaddr* addr, socklen_t _addr_len);
		void	run_cgi_script(const std::string& script_path, Epoll& epoll);

	private:
		socklen_t			addr_len;
		struct sockaddr_in	client_addr;
		socklen_t			server_addr_len;
		struct sockaddr_storage	server_addr;
		Server				*server;
		std::string			server_host;
		std::string			server_port;
		std::string			host;
		std::string			port;
		std::string			buffer;
		std::string			res_buffer;
		HttpRequest			request;
		HttpResponse		response;
		time_t				expiry;
		bool				server_error;
		bool				timed_out;
		enum StatusCode
		{
			OK = 200,
			MOVED_PERMANENTLY = 301,
			MOVED_TEMPORARILY = 302,
			BAD_REQUEST = 400,
			RESOURCE_NOT_FOUND = 404,
			METHOD_NOT_ALLOWED = 405,
			REQUEST_TIMEOUT = 408,
			CONTENT_LENGTH_MISSING = 411,
			PAYLOAD_TOO_LARGE = 413,
			INTERNAL_SERVER_ERROR = 500
		};
		static const std::size_t	MAX_HEADER_SIZE = 8192;

		static std::string	readFileContent(const std::string &path);
		void		sendErrorResponse(StatusCode status_code,
                                        const std::string &status_text,
                                        const std::string &error_file,
                                        const std::vector<std::string> &allow_methods);
		static std::string	makeIndexof(std::string const &path_dir, std::string const &uri);
		static std::string	getFileExtension(const std::string& filepath);
};


#endif
