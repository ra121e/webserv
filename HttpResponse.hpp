/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: athonda <athonda@student.42singapore.sg    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/07 20:58:04 by athonda           #+#    #+#             */
/*   Updated: 2025/08/08 16:47:51 by athonda          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPRESPONSE_HPP
# define HTTPRESPONSE_HPP

# include <string>
# include <map>
# include <sstream>

class HttpResponse
{
	public:
		int									status_code;
		std::string							status_message;
		std::map<std::string, std::string>	headers;
		std::string							body;

		HttpResponse();
		HttpResponse(HttpResponse const &other);
		HttpResponse	&operator=(HttpResponse const &other);
		~HttpResponse();

		void	addHeader(std::string const &key, std::string const &value);
		void	setBody(std::string const &body_content, std::string const &content_type);
		std::string	makeString() const;
};


#endif
