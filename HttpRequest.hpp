/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgoh <cgoh@student.42singapore.sg>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/05 16:36:16 by athonda           #+#    #+#             */
/*   Updated: 2025/08/13 20:09:56 by cgoh             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPREQUEST_HPP
# define HTTPREQUEST_HPP

#include <cstddef>
#include <string>
#include <map>

class HttpRequest
{
	public:
		HttpRequest();

		std::string	method;
		std::string	uri;
		std::string	version;

		std::map<std::string, std::string>	headers;
		std::string	body;

		bool	is_header_parse;
		bool	waiting_for_body;
		bool	body_too_large;
		bool	is_bad;
		bool	content_length_missing;
		size_t	header_end_pos;
		size_t	content_length;
	private:

};

#endif
