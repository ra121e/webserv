/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgoh <cgoh@student.42singapore.sg>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/07 20:58:04 by athonda           #+#    #+#             */
/*   Updated: 2025/08/11 18:44:02 by cgoh             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPRESPONSE_HPP
# define HTTPRESPONSE_HPP

# include <string>
# include <map>
# include <sstream>

class HttpResponse
{
	private:
		static const int OK = 200;
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
