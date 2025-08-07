/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientConnection.hpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgoh <cgoh@student.42singapore.sg>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/05 16:52:27 by athonda           #+#    #+#             */
/*   Updated: 2025/08/07 18:09:29 by cgoh             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENTCONNECTION_HPP
# define CLIENTCONNECTION_HPP

# include "HttpRequest.hpp"

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

		void	append_to_buffer(char const *data, size_t size);
		bool	parse_request();
//		bool	parse_headers();
//		bool	parse_body();

	private:
		int			fd;
		std::string	buffer;
		HttpRequest	request;

};


#endif
