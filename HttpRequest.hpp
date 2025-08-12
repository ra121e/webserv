/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgoh <cgoh@student.42singapore.sg>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/05 16:36:16 by athonda           #+#    #+#             */
/*   Updated: 2025/08/12 16:26:52 by cgoh             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPREQUEST_HPP
# define HTTPREQUEST_HPP

#include <string>
#include <map>

class HttpRequest
{
	public:
		HttpRequest();
		HttpRequest(HttpRequest const &other);
		HttpRequest	&operator=(HttpRequest const &other);
		~HttpRequest();

		std::string	method;
		std::string	uri;
		std::string	version;

		std::map<std::string, std::string>	headers;
		std::string	body;

		bool	is_header_parse;
	private:

};

#endif
