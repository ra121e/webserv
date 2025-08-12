/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientConnection.hpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgoh <cgoh@student.42singapore.sg>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/05 16:52:27 by athonda           #+#    #+#             */
/*   Updated: 2025/08/12 16:27:18 by cgoh             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENTCONNECTION_HPP
# define CLIENTCONNECTION_HPP

# include "HttpRequest.hpp"
# include "HttpResponse.hpp"
# include "Server.hpp"
#include <netdb.h>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>

class ClientConnection
{
	public:
		ClientConnection();
		ClientConnection(int server_fd, Server *srv);
		ClientConnection(ClientConnection const &other);
		ClientConnection	&operator=(ClientConnection const &other);
		~ClientConnection();

		int	getFd() const;
		const HttpRequest	&getRequest() const;
		const std::string	&getBuffer() const;

		const HttpResponse	&getResponse() const;
		const std::string	&getResponseBuffer() const;
		std::string getHost() const;
		Server	*getServer() const;

		void	appendToBuffer(char const *data, size_t size);
		void	parseRequest();
		void	makeResponse();
		bool	sendResponse();
		void	retrieveHost();

	private:
		socklen_t			addr_len;
		struct sockaddr_in	client_addr;
		int					fd;
		Server				*server;
		char				host[NI_MAXHOST];
		std::string			buffer;
		std::string			res_buffer;
		HttpRequest			request;
		HttpResponse		response;
		static const int OK = 200;
		static const int BAD_REQUEST = 400;
		static const int RESOURCE_NOT_FOUND = 404;
		static const int METHOD_NOT_ALLOWED = 405;
		static const int INTERNAL_SERVER_ERROR = 500;
		
		std::string       readFileContent(const std::string &path) const;
		void              sendErrorResponse(int status_code,
                                        const std::string &status_text,
                                        const std::string &error_file,
                                        const std::vector<std::string> &allow_methods);
};


#endif
