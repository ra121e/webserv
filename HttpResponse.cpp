/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgoh <cgoh@student.42singapore.sg>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/08 09:02:49 by athonda           #+#    #+#             */
/*   Updated: 2025/08/11 18:42:22 by cgoh             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpResponse.hpp"
#include <sstream>

void	HttpResponse::addHeader(std::string const &key, std::string const &value)
{
	headers[key] = value;
}

void	HttpResponse::setBody(std::string const &body_content, std::string const &content_type)
{
	body = body_content;
	addHeader("Content-Type", content_type);

	std::ostringstream	oss;
	oss << body.size();
	addHeader("Content-Length", oss.str());
}

std::string	HttpResponse::makeString() const
{
	std::stringstream	ss;

	ss << "HTTP/1.1 " << status_code << " " << status_message << "\r\n";
	ss << "Connection: close\r\n";
	if (headers.find("Content-Length") == headers.end())
		ss << "Content-Length: " << body.size() << "\r\n";

	std::map<std::string, std::string>::const_iterator it = headers.begin();
	for (; it != headers.end(); ++it)
	{
		ss << it->first << ": " << it->second << "\r\n";
	}

	ss << "\r\n";
	ss << body;

	return (ss.str());
}
