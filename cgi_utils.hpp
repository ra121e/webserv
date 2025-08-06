/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cgi_utils.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgoh <cgoh@student.42singapore.sg>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/29 10:45:08 by apoh              #+#    #+#             */
/*   Updated: 2025/08/06 18:06:18 by cgoh             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGI_UTILS_HPP
# define CGI_UTILS_HPP

# include <iostream>
# include <fcntl.h>
# include <errno.h>
# include <string.h>
# include <stdlib.h>
# include <unistd.h>
# include <cstdlib>
# include <cerrno>
# include <cstring>
# include <sys/wait.h>
# include <sys/stat.h>
# include <sstream>
# include <fstream>
# include <map>
# include <vector>
# include <algorithm>
# include <sys/select.h>
# include <signal.h>

# define BUFFER_SIZE 1024

const size_t REQUEST_PAYLOAD_SIZE = 2048;

bool	is_forbidden(const std::string &path); // File Permission checks //
bool	is_not_found(const std::string &path); // File checks //

bool	is_method_allowed(const std::string &method, const std::vector<std::string> &allowed_methods);
	// Http request method checks //

std::string read_file(const std::string &filepath); // Function to read file //

void	send_error_response(int status_code, const std::string &status_text,
		const std::string &error_file,
		const std::vector<std::string> &allow_methods); // Writing error response //

void	send_successful_response(const std::string &file_path, const std::string &content_type);
	// Writing successful response //
bool	is_cgi_script(const std::string &path);
int		create_pipe(int fds[2]);
size_t	ft_strlen(const char *s);
char	*ft_strdup(const char *s);
size_t	ft_strlcpy(char *dest, const char *src, size_t size);
char	**map_to_envp(const std::map<std::string, std::string> &env_map);
void	free_envp(char **envp);
ssize_t	write_all(int fd, const char *buf, size_t size);

#endif
