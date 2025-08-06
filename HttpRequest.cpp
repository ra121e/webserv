/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: athonda <athonda@student.42singapore.sg    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/05 16:46:37 by athonda           #+#    #+#             */
/*   Updated: 2025/08/06 11:40:16 by athonda          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpRequest.hpp"

HttpRequest::HttpRequest():
	is_header_parse(false),
	is_parse_complete(false)
{}

HttpRequest::HttpRequest(HttpRequest const &other):
	method(other.method),
	uri(other.uri),
	version(other.version),
	headers(other.headers),
	body(other.body),
	is_header_parse(other.is_header_parse),
	is_parse_complete(other.is_parse_complete)
{}

HttpRequest	&HttpRequest::operator=(HttpRequest const &other)
{
	if (this != &other)
	{
		this->method = other.method;
		this->uri = other.uri;
		this->version = other.version;
		this->headers = other.headers;
		this->body = other.body;
		this->is_header_parse = other.is_header_parse;
		this->is_parse_complete = other.is_parse_complete;
	}
	return (*this);
}

HttpRequest::~HttpRequest()
{}
