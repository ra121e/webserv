/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientConnection.hpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: athonda <athonda@student.42singapore.sg    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/05 16:52:27 by athonda           #+#    #+#             */
/*   Updated: 2025/08/09 22:02:46 by athonda          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENTCONNECTION_HPP
# define CLIENTCONNECTION_HPP

# include "HttpRequest.hpp"
# include "HttpResponse.hpp"
# include "Server.hpp"
#include <netdb.h>
#include <netinet/in.h>
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

		void	appendToBuffer(char const *data, size_t size);
		bool	parseRequest();
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
};


#endif
