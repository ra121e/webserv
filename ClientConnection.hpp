/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientConnection.hpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgoh <cgoh@student.42singapore.sg>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/05 16:52:27 by athonda           #+#    #+#             */
/*   Updated: 2025/08/08 15:43:36 by cgoh             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENTCONNECTION_HPP
# define CLIENTCONNECTION_HPP

# include "HttpRequest.hpp"
# include "HttpResponse.hpp"
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>

class ClientConnection
{
	public:
		ClientConnection();
		ClientConnection(int socket_fd);
		ClientConnection(ClientConnection const &other);
		ClientConnection	&operator=(ClientConnection const &other);
		~ClientConnection();

		int	getFd() const;
		const HttpRequest	&getRequest() const;
		const std::string	&getBuffer() const;

		const HttpResponse	&getResponse() const;
		const std::string	&getResponseBuffer() const;

		void	append_to_buffer(char const *data, size_t size);
		bool	parse_request();
		void	makeResponse();
		void	retrieveHost();

	private:
		socklen_t		addr_len;
		struct sockaddr_in	client_addr;
		int				fd;
		char			host[NI_MAXHOST];
		std::string		buffer;
		std::string		res_buffer;
		HttpRequest		request;
		HttpResponse	response;


};


#endif
