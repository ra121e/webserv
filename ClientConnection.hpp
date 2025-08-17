/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientConnection.hpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgoh <cgoh@student.42singapore.sg>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/05 16:52:27 by athonda           #+#    #+#             */
/*   Updated: 2025/08/17 22:34:21 by cgoh             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENTCONNECTION_HPP
# define CLIENTCONNECTION_HPP

# include "HttpRequest.hpp"
# include "HttpResponse.hpp"
# include "cgi_handler.hpp"
# include "Server.hpp"
#include <ctime>
#include <netdb.h>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include "BaseFile.hpp"

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
		time_t	getExpiry() const;

		void	appendToBuffer(char const *data, size_t size);
		bool	parseRequest();
		void	makeResponse();
		bool	sendResponse();
		void	retrieveHost();

	private:
		socklen_t			addr_len;
		struct sockaddr_in	client_addr;
		Server				*server;
		std::string			host;
		std::string			buffer;
		std::string			res_buffer;
		HttpRequest			request;
		HttpResponse		response;
		bool				server_error;
		time_t				expiry;
		enum StatusCode
		{
			OK = 200,
			MOVED_PERMANENTLY = 301,
			MOVED_TEMPORARILY = 302,
			BAD_REQUEST = 400,
			RESOURCE_NOT_FOUND = 404,
			METHOD_NOT_ALLOWED = 405,
			PAYLOAD_TOO_LARGE = 413,
			INTERNAL_SERVER_ERROR = 500
		};

		std::string	readFileContent(const std::string &path) const;
		void		sendErrorResponse(StatusCode status_code,
                                        const std::string &status_text,
                                        const std::string &error_file,
                                        const std::vector<std::string> &allow_methods);
		static std::string	makeIndexof(std::string const &path_dir, std::string const &uri);
		static std::string	getFileExtension(const std::string& filepath);
};


#endif
