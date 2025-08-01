/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cgi_utils.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: apoh <marvin@42.fr>                        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/29 10:45:10 by apoh              #+#    #+#             */
/*   Updated: 2025/07/29 10:45:11 by apoh             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "cgi_utils.hpp"

bool	is_forbidden(const std::string &path) // Error 403 Forbidden //
{
	int	fd = open(path.c_str(), O_RDONLY);
	if (fd == -1)
	{
		if (errno == EACCES) // EACCES means permission denied //
			return (true);
	}
	else
		close(fd);
	return (false);
}

bool	is_not_found(const std::string &path) // Error 404 not found //
{
	int	fd = open(path.c_str(), O_RDONLY);
	if (fd == -1)
	{
		if (errno == ENOENT) // ENOENT means no such file //
			return (true);
	}
	else
		close(fd);
	return (false);
}

std::string read_file(const std::string &filepath)
{
	std::ifstream file(filepath.c_str()); // input stream //
	std::ostringstream ss; // output stream // 
	ss << file.rdbuf(); // read the whole file buffer data into output stream //
	return ss.str(); // returns content to a string // 
}

void	send_error_response(int status_code, const std::string &status_text,
		const std::string &error_file,
		const std::vector<std::string> &allow_methods)
{
	std::string error_content = read_file(error_file); // read data first //
	std::cout << "HTTP/1.1 " << status_code << " " << status_text << "\r\n"; // apply status and the text associated with the error HTTP/1.1 requires \r\n to be returned.// 
	std::cout << "Content-Type: text/html\r\n";
	std::cout << "Content-Length: " << error_content.size() << "\r\n";
	if (!allow_methods.empty()) // if methods are not empty then come here to print it out //
	{
		std::cout << "Allow: ";
		for (size_t i = 0; i < allow_methods.size();++i)
		{
			std::cout << allow_methods[i];
			if (i + 1 < allow_methods.size())
				std::cout << ", ";
		}
		std::cout << "\r\n";
	}
	std::cout << "\r\n";
	std::cout << error_content; // print out the content of the error for debugging //
}

bool	is_method_allowed(const std::string &method, const std::vector<std::string> &allowed_methods) // check whether it belongs to GET, POST, DELETE // 
{
	return std::find(allowed_methods.begin(), allowed_methods.end(), method)
		!= allowed_methods.end();
}

void	send_successful_response(const std::string &file_path, const std::string &content_type) // haven used this function yet but should be useful // 
{
	std::string file_content = read_file(file_path); // Implement this to read the file into a string
	std::cout << "HTTP/1.1 200 OK\r\n";
	std::cout << "Content-Type: " << content_type << "\r\n";
	std::cout << "Content-Length: " << file_content.size() << "\r\n";
	std::cout << "\r\n"; // End of headers
	std::cout << file_content;
}
