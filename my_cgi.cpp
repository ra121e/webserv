/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   my_cgi.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: apoh <marvin@42.fr>                        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/29 13:03:33 by apoh              #+#    #+#             */
/*   Updated: 2025/07/29 13:03:34 by apoh             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "cgi_utils.hpp"

int	delete_file(const char *filepath, char **cgi_envp) // needed to use execve to delete a file //
{
	pid_t	pid = fork();
	if (pid == -1)
	{
		std::cerr << "fork failed: " << strerror(errno) << std::endl;
		return (-1);
	}
	if (pid == 0)
	{
		char	*argv[] = {
			const_cast<char*>("/bin/rm"), // first argument // 
			const_cast<char*>("-r"), // next one // 
			const_cast<char*>(filepath), // 3rd one // 
			NULL // null terminated // 
			};
		execve("/bin/rm", argv, cgi_envp);
		exit(EXIT_FAILURE); // because if success it will not come here //
	}
	else
	{
		int	status = 0;
		waitpid(pid, &status, 0);
		if (WIFEXITED(status) && WEXITSTATUS(status) == 0) // WIFEXITED whether it exits successfully, WEXITSTAUS gets the child exit code //
			return (0);
		else
			return (-1);
	}
}

int	read_fully(int fd, char *buffer, size_t length) // handling partial reads // 
{
	size_t	total_read = 0;
	
	while (total_read < length)
	{	
		ssize_t bytes_read = read(fd, buffer + total_read, length - total_read);
		if (bytes_read == -1)
		{
			if (errno == EINTR)
				continue ;
			perror("read failed");
			return (-1);
		}
		else if (bytes_read == 0)
			break ;
		total_read += bytes_read;
	}
	return (total_read);
}

int	main(int ac, char **av, char **envp)
{
	(void)ac;
	(void)av;
	sleep(10); // this is to stimulate timeout for handler // 
	std::vector<std::string> allowed_methods; // create a vector to hold allowed methods // 
	allowed_methods.push_back("GET");
	allowed_methods.push_back("POST");
	allowed_methods.push_back("DELETE");
	std::string file_path = "/home/apoh/Documents/test/webserv/cgi-bin/forbidden.txt"; // file path to delete // 
	const char	*method = std::getenv("REQUEST_METHOD");
	if (method == NULL) // Check method //
	{
		std::cout << "Content-Type: text/plain\r\n\r\n";
		std::cout << "No REQUEST_METHOD set." << std::endl;
		return (1);
	}
	if (!is_method_allowed(method, allowed_methods)) // check if allowed method //
	{	
		send_error_response(405, "Method Not Allowed", "error_405.html", allowed_methods);
		return (1);
	}
	std::cout << "Printing Content-Type: text/plain\r\n\r\n";
	if (std::string(method) == "GET") // if method == GET //
	{
		const char *query = std::getenv("QUERY_STRING"); // Process query string //
		if (query == NULL)
		{
			std::cout << "GET request but no QUERY_STRING" << std::endl;
			return (1);
		}
		else
		{
			std::cout << "GET query string: " << query << std::endl;
			return (0);
		}
	}
	else if (std::string(method) == "POST") // if method == POST //
	{
		const char	*content_length_str = std::getenv("CONTENT_LENGTH"); // Read Content Length, use atoi to convert to a number and then read the data fully using a loop // 
		int		content_length = 0;
		if (content_length_str != NULL)
			content_length = std::atoi(content_length_str);
		if (content_length > 0)
		{
			char *buffer = new char[content_length + 1]; // got the size already //
			ssize_t	total_read = read_fully(STDIN_FILENO, buffer, content_length);
			if (total_read == -1)
			{
				std::cerr << "Error reading POST data" << std::endl;
				delete[] buffer;
				return (1);
			}
			buffer[total_read] = '\0';
			std::cout << "POST data: " << buffer << std::endl;
			delete[] buffer;
			return (0);
		}
		else
		{
			std::cout << "NO POST data" << std::endl;
			return (0);
		}
	}
	else if (std::string(method) == "DELETE") // See if file has permissions, can be found, Any data in it while doing execve or internal server error //
	{
		std::vector<std::string>	empty_methods;
		if (is_not_found(file_path))
		{
			send_error_response(404, "Not Found", "error_404.html", empty_methods);
			return (1);
		}
		if (is_forbidden(file_path))
		{
			send_error_response(403, "Forbidden", "error_403.html", empty_methods);
			return (1);
		}
		if (delete_file(file_path.c_str(), envp) == 0)
		{
			std::cout << "HTTP/1.1 204 No Content\r\n\r\n";
			return (0);
		}
		else
		{
			send_error_response(500, "Internal Server Error", "error_500.html",
			empty_methods);
			return (1);
		}
	}
	else
	{
		std::cout << "Unsupported method: " << method << std::endl;
		return (1);
	}
	return (0);
}
