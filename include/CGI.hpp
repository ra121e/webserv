#ifndef CGI_HPP
#define CGI_HPP
#include "Pipe.hpp"
#include <cstddef>
#include <ctime>
#include <map>
#include <string>
#include <sys/types.h>
#include <utility>

// Forward declaration to avoid circular dependency
class ClientConnection;
class Epoll;

class CGI
{
private:
	Pipe	server_write_cgi_read_pipe; // pipe for stdin
	Pipe	server_read_cgi_write_pipe; // pipe for stdout
	std::map<std::string, std::string> env_map; // environment variables for the CGI script
	char	**envp; // environment pointer for execve
	ClientConnection* client; // Pointer to the associated ClientConnection
	pid_t	pid;
	bool	finished;

	CGI(const CGI& other);
	CGI& operator=(const CGI& other);
public:
	CGI(ClientConnection* client, std::pair<std::string, std::string> cgi_params[], size_t params_size);
	~CGI();
	int		get_server_write_fd() const;
	int		get_server_read_fd() const;
	int		get_client_fd() const;
	void	set_client_server_error(bool error);
	void	set_client_cgi_timed_out(bool timeout);
	void	append_to_client_res_buffer(const char* data, size_t size);
	const std::string&	get_client_buffer() const;
	void	convert_env_map_to_envp();
	void	execute_cgi();
	void	get_cgi_response();
	void	close_pipes();
	void	close_server_write_fd();
	void	setPid(pid_t _pid);
	pid_t	getPid() const;
	void	make_client_response(Epoll& epoll);
	void	setFinished(bool val);
	bool	isFinished() const;
};

#endif
