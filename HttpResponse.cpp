/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: athonda <athonda@student.42singapore.sg    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/08 09:02:49 by athonda           #+#    #+#             */
/*   Updated: 2025/08/08 11:35:36 by athonda          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpResponse.hpp"
#include <sstream>

HttpResponse::HttpResponse():
	status_code(200),
	status_message("OK")
{}

HttpResponse::HttpResponse(HttpResponse const &other):
	status_code(other.status_code),
	status_message(other.status_message),
	headers(other.headers),
	body(other.body)
{}

HttpResponse	&HttpResponse::operator=(HttpResponse const &other)
{
	if (this != &other)
	{
		this->status_code = other.status_code;
		this->status_message = other.status_message;
		this->headers = other.headers;
		this->body = other.body;
	}
	return (*this);
}

HttpResponse::~HttpResponse()
{}

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