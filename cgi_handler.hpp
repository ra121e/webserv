/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cgi_handler.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: apoh <marvin@42.fr>                        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/08 12:56:04 by apoh              #+#    #+#             */
/*   Updated: 2025/08/08 12:56:11 by apoh             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGI_HANDLER_HPP
# define CGI_HANDLER_HPP

# include "cgi_utils.hpp"
# include <sys/types.h>
# include <string>
# include <map>
# include "ClientConnection.hpp"
# include "Network.hpp"
# include <sys/epoll.h>

bool is_cgi_script(const std::string &path);
int create_pipe(int fds[2]);
size_t ft_strlen(const char *s);
char *ft_strdup(const char *s);
size_t ft_strlcpy(char *dest, const char *src, size_t size);
char **map_to_envp(const std::map<std::string, std::string> &env_map);
void free_envp(char **envp);
ssize_t write_all(int fd, const char *buf, size_t size);
int run_cgi_script(ClientConnection &client, const std::string &script_path);

#endif
