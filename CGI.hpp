#ifndef CGI_HPP
#define CGI_HPP
#include "Pipe.hpp"
#include <ctime>
#include <map>
#include <string>
#include "ClientConnection.hpp"

class CGI
{
private:
	Pipe	server_write_cgi_read_pipe; // pipe for stdin
	Pipe	server_read_cgi_write_pipe; // pipe for stdout
	std::map<std::string, std::string> env_map; // environment variables for the CGI script
	char	**envp; // environment pointer for execve
	ClientConnection* client; // Pointer to the associated ClientConnection
	time_t	expiry;

	CGI(const CGI& other);
	CGI& operator=(const CGI& other);
public:
	CGI(ClientConnection* client);
	~CGI();
	int		get_server_write_fd() const;
	int		get_server_read_fd() const;
	void	add_env(const std::string &key, const std::string &value);
	void	convert_env_map_to_envp();
	void	execute_cgi();
	void	get_cgi_response();
	void	close_pipes();
};

#endif
