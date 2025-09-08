/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgoh <cgoh@student.42singapore.sg>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/05 16:36:16 by athonda           #+#    #+#             */
/*   Updated: 2025/09/08 20:23:18 by cgoh             ###   ########.fr       */
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
		std::string	extension;
		std::string	version;
		std::map<std::string, std::string>	headers;
		std::string	body;
		bool	is_header_parse;
		bool	waiting_for_body;
		bool	body_too_large;
		bool	is_bad;
		bool	content_length_missing;
		bool	forward_to_cgi;
		size_t	header_end_pos;
		size_t	content_length;

		std::string getQueryString() const;
		std::string getContentLength() const;
		std::string getContentType() const;
		std::string getScriptName() const;
		std::string getCookie() const;
		std::string getPathInfo() const;
		std::string getCookieValue(const std::string& cookie_name) const;
	private:

};

#endif
