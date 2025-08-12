/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cgi_handler.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgoh <cgoh@student.42singapore.sg>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/08 12:56:04 by apoh              #+#    #+#             */
/*   Updated: 2025/08/09 16:38:44 by cgoh             ###   ########.fr       */
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

class ClientConnection;

bool is_cgi_script(const std::string &path);
int create_pipe(int fds[2]);
size_t ft_strlen(const char *s);
char *ft_strdup(const char *s);
size_t ft_strlcpy(char *dest, const char *src, size_t size);
char **map_to_envp(const std::map<std::string, std::string> &env_map);
void free_envp(char **envp);
ssize_t write_all(int fd, const char *buf, size_t size);
int run_cgi_script(ClientConnection &client, const std::string &script_path);
std::string getQueryString(const std::string &uri);
std::string getContentLength(const std::map<std::string, std::string> &headers);
std::string getContentType(const std::map<std::string, std::string> &headers);
std::string getScriptName(const std::string &uri);
std::string getUserAgent(const std::map<std::string, std::string> &headers);
std::string getCookie(const std::map<std::string, std::string> &headers);
std::string getReferer(const std::map<std::string, std::string> &headers);
std::string getPathInfo(const std::string &script_path);
int run_cgi_script(ClientConnection &client, const std::string &script_path, const Network &network);

#endif
