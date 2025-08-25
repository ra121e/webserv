#ifndef CGI_HPP
#define CGI_HPP
#include "Pipe.hpp"
#include <map>
#include <string>

class CGI
{
private:
	Pipe	pipe_stdin; // pipe for stdin
	Pipe	pipe_stdout; // pipe for stdout
	std::map<std::string, std::string> env_map; // environment variables for the CGI script
	char	**envp; // environment pointer for execve

	CGI(const CGI& other);
	CGI& operator=(const CGI& other);
public:
	CGI();
	~CGI();
	void	add_env(const std::string &key, const std::string &value);
	void	convert_env_map_to_envp();
	void	execute_cgi();
};

#endif
