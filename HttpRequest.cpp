/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgoh <cgoh@student.42singapore.sg>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/05 16:46:37 by athonda           #+#    #+#             */
/*   Updated: 2025/08/30 20:51:31 by cgoh             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpRequest.hpp"

HttpRequest::HttpRequest():
	is_header_parse(false), waiting_for_body(false), body_too_large(false),
	is_bad(false), content_length_missing(false), forward_to_cgi(false),
	header_end_pos(0), content_length(0)
{}
