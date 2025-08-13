/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgoh <cgoh@student.42singapore.sg>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/05 16:46:37 by athonda           #+#    #+#             */
/*   Updated: 2025/08/13 17:41:17 by cgoh             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpRequest.hpp"

HttpRequest::HttpRequest():
	is_header_parse(false), waiting_for_body(false), header_end_pos(0), content_length(0)
{}
HttpRequest::HttpRequest(HttpRequest const &other):
	method(other.method),
	uri(other.uri),
	version(other.version),
	headers(other.headers),
	body(other.body),
	is_header_parse(other.is_header_parse),
	waiting_for_body(other.waiting_for_body),
	header_end_pos(other.header_end_pos),
	content_length(other.content_length)
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
		this->waiting_for_body = other.waiting_for_body;
		this->header_end_pos = other.header_end_pos;
		this->content_length = other.content_length;
	}
	return (*this);
}

HttpRequest::~HttpRequest()
{}
