/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgoh <cgoh@student.42singapore.sg>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/05 16:46:37 by athonda           #+#    #+#             */
/*   Updated: 2025/09/08 20:23:54 by cgoh             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/HttpRequest.hpp"
#include <string>

HttpRequest::HttpRequest():
	is_header_parse(false), waiting_for_body(false), body_too_large(false),
	is_bad(false), content_length_missing(false), forward_to_cgi(false),
	header_end_pos(0), content_length(0)
{}

std::string HttpRequest::getQueryString() const // getting the query string on GET method //
{
	std::size_t pos = uri.find('?'); // ? is the point where the query string starts //
	if (pos == std::string::npos)
		return "";
	return uri.substr(pos + 1);
}

std::string HttpRequest::getContentLength() const // Under the headers tag, retrieve Content-Length //
{
	std::map<std::string, std::string>::const_iterator it = headers.find("Content-Length");
	if (it != headers.end())
		return it->second;
	return "";
}

std::string HttpRequest::getContentType() const // Under the headers tag, retrieve Content-Type //
{
	std::map<std::string, std::string>::const_iterator it = headers.find("Content-Type");
	if (it != headers.end())
		return it->second;
	return "";
}

std::string HttpRequest::getScriptName() const // script name is the one before the ? //
{
	std::size_t pos = uri.find('?');
	if (pos == std::string::npos)
		return uri; // if no question mark, then its the whole script name //
	return uri.substr(0, pos);
}

std::string HttpRequest::getCookie() const // under the headers tag, retrieve Cookie //
{
	std::map<std::string, std::string>::const_iterator it = headers.find("Cookie");	
	if (it != headers.end())
		return it->second;
	return "";
}

std::string HttpRequest::getPathInfo() const
{
	std::string new_str = uri;
	size_t pos = new_str.find('?');
	if (pos != std::string::npos)
		new_str.erase(pos);
	return (new_str);
}

std::string	HttpRequest::getCookieValue(const std::string& cookie_name) const
{
	std::map<std::string, std::string>::const_iterator it = headers.find("Cookie");
	if (it == headers.end())
	{
		return "";
	}
	const std::string&	cookie_header = it->second;
	size_t start = cookie_header.find(cookie_name + "=");
	if (start == std::string::npos)
	{
		return "";
	}
	start += cookie_name.length() + 1; // Move past "cookie_name="
	size_t end = cookie_header.find(';', start);
	if (end == std::string::npos)
	{
		end = cookie_header.length();
	}
	return cookie_header.substr(start, end - start);
}
