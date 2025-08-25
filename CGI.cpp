#include "CGI.hpp"
#include <cstddef>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <stdexcept>
#include <cerrno>

CGI::CGI() : envp(NULL)
{
}

CGI::~CGI()
{
	if (envp)
	{
		for (std::size_t i = 0; envp[i]; ++i)
		{
			delete[] envp[i];
		}
		delete[] envp;
	}
	envp = NULL;
}

void	CGI::add_env(const std::string &key, const std::string &value)
{
	env_map[key] = value;
}

void	CGI::convert_env_map_to_envp()
{
	envp = new char*[env_map.size() + 1]; // run_cgi function for env_map //
	std::size_t	index = 0;

	for (std::map<std::string, std::string>::const_iterator it = env_map.begin();
		it != env_map.end(); ++it) // const iterator so that values doesnt change //
	{
		std::string pair = it->first + "=" + it->second;
		char *cstr = new char[pair.size() + 1];
		strlcpy(cstr, pair.c_str(), pair.size() + 1); // convert pair to C string so you can copy //
		envp[index++] = cstr;
	}
	envp[index] = NULL;
}

void	CGI::execute_cgi()
{
	if (dup2(server_write_cgi_read_pipe[STDIN_FILENO].getFd(), STDIN_FILENO) == -1) // dup2 the readend of stdin since thats needed // 
	{
		throw std::runtime_error("dup2 failed for stdin: " + std::string(strerror(errno)));
	}
	if (dup2(server_read_cgi_write_pipe[STDOUT_FILENO].getFd(), STDOUT_FILENO) == -1) // dup2 the write end of stdout as whatever the child needs to output goes into that area // 
	{
		throw std::runtime_error("dup2 failed for stdout: " + std::string(strerror(errno)));
	}
	std::string	str_path = "cgi-bin" + env_map["PATH_INFO"];
	const char *cgi_path = str_path.c_str();
	const char	*argv[] = {cgi_path, NULL};
	execve(cgi_path, const_cast<char *const *>(argv), envp);
	throw std::runtime_error("execve failed: " + std::string(strerror(errno)));
}

void	CGI::get_cgi_response()
{
	
	if (env_map["REQUEST_METHOD"] == "POST")
	{
		const std::string &body = req.body;
		if (!body.empty())
		{
			if (write_all(server_write_cgi_read_pipe[1], body.c_str(), body.size()) == -1) // if post method, write data all to stdin // 
			{
				perror("Write to CGI stdin failed");
				kill(pid, SIGKILL); // if failure, kill off the child and close all the pipes and free cgi_envp memory // 
				close(server_write_cgi_read_pipe[1]);
				close(server_read_cgi_write_pipe[0]);
				free_envp(cgi_envp);
				return (-1);
			}
		}
	}
	close(server_write_cgi_read_pipe[1]); // once data is written to stdin pipe, close off the pipe since no longer used // 
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
