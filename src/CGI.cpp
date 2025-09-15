#include "../include/CGI.hpp"
#include <cstddef>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <stdexcept>
#include <cerrno>
#include <bsd/string.h>
#include "../include/ClientConnection.hpp"
#include <iostream>
#include <utility>

CGI::CGI(const SharedPointer<ClientConnection>& _client, std::pair<std::string, std::string> cgi_params[],
	size_t params_size, time_t expiry) :
	BaseExpiration(expiry),
	env_map(cgi_params, cgi_params + params_size),
	client(_client),
	pid(0)
{
}

char**	CGI::convert_env_map_to_envp()
{
	char** envp = new char*[env_map.size() + 1]; // run_cgi function for env_map //
	std::size_t	index = 0;

	for (std::map<std::string, std::string>::const_iterator it = env_map.begin();
		it != env_map.end(); ++it) // const iterator so that values doesnt change //
	{
		const std::string& pair = it->first + "=" + it->second;
		char *cstr = new char[pair.size() + 1];
		strlcpy(cstr, pair.c_str(), pair.size() + 1); // convert pair to C string so you can copy //
		envp[index++] = cstr;
	}
	envp[index] = NULL;
	return envp;
}

void	CGI::execute_cgi()
{
	if (dup2(server_write_cgi_read_pipe[STDIN_FILENO].getFd(), STDIN_FILENO) == -1)
	// dup2 the readend of stdin since thats needed // 
	{
		throw std::runtime_error("dup2 failed for stdin: " + std::string(strerror(errno)));
	}
	if (dup2(server_read_cgi_write_pipe[STDOUT_FILENO].getFd(), STDOUT_FILENO) == -1)
	// dup2 the write end of stdout as whatever the child needs to output goes into that area // 
	{
		throw std::runtime_error("dup2 failed for stdout: " + std::string(strerror(errno)));
	}
	std::string	str_path = "cgi-bin" + env_map["PATH_INFO"];
	const char *cgi_path = str_path.c_str();
	const char	*argv[] = {cgi_path, NULL};
	char	**envp = convert_env_map_to_envp();
	execve(cgi_path, const_cast<char *const *>(argv), envp);
	std::cout << "cgi_path: " << cgi_path << '\n';
	throw std::runtime_error("execve failed: " + std::string(strerror(errno)));
}

void	CGI::close_pipes()
{
	server_write_cgi_read_pipe[STDIN_FILENO].closeFd();
	server_read_cgi_write_pipe[STDOUT_FILENO].closeFd();
}

int	CGI::get_server_write_fd() const
{
	return server_write_cgi_read_pipe[STDOUT_FILENO].getFd();
}

int	CGI::get_server_read_fd() const
{
	return server_read_cgi_write_pipe[STDIN_FILENO].getFd();
}

void	CGI::close_server_write_fd()
{
	server_write_cgi_read_pipe[STDOUT_FILENO].closeFd();
}

void	CGI::setPid(pid_t _pid)
{
	pid = _pid;
}

pid_t	CGI::getPid() const
{
	return pid;
}

int	CGI::get_client_fd() const
{
	return client->getFd();
}

void	CGI::set_client_server_error(bool error)
{
	client->setServerError(error);
}

void	CGI::set_client_cgi_timed_out(bool timeout)
{
	client->setCgiTimedOut(timeout);
}

void	CGI::append_to_client_res_buffer(const char* data, size_t size)
{
	client->appendToResBuffer(data, size);
}

const std::string&	CGI::get_client_buffer() const
{
	return client->getBuffer();
}

void	CGI::make_client_response(Epoll& epoll)
{
	client->makeResponse(epoll, getClientConnection());
}

const SharedPointer<ClientConnection>& CGI::getClientConnection() const
{
	return client;
}
