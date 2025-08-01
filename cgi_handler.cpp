/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cgi.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: apoh <marvin@42.fr>                        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/17 17:09:41 by apoh              #+#    #+#             */
/*   Updated: 2025/07/17 17:09:42 by apoh             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "cgi_utils.hpp"

bool	is_cgi_script(const std::string &path) // checks if path starts with /cgi/cbin or is it a .cgi file. Have not used this yet but it would be a good function to keep // 
{
	if (path.substr(0, 9) == "/cgi-bin/")
		return (true);
	std::string extension = ".cgi"; 
	if (path.length() >= extension.length() && path.substr(path.length() - extension.length()) == extension)
		return (true);
	return (false);
}

int	create_pipe(int fds[2]) // pipe creation. One for parent one for child, fd[0] is read end fd[1] is write in //
{
	if (pipe(fds) == -1)
	{
		perror("pipe failed");
		return (-1);
	}
	return (0);
}

int	ft_strlen(const char *s) // if using libft, this function can throw away //
{
	if (s == NULL)
		return (0);
	int	len = 0;
	while (s[len] != '\0')
		len++;
	return (len);
}

char	*ft_strdup(const char *s) // same, if using libft, this function can throw away //
{
	if (s == NULL)
		return (NULL);
	int	len = ft_strlen(s);
	char	*dup = (char *)malloc(len + 1);
	int	i = 0;
	
	while (i < len)
	{
		dup[i] = s[i];
		i++;
	}
	dup[i] = '\0';
	return (dup);
}

size_t	ft_strlcpy(char *dest, const char *src, size_t size) // same, if using libft, this function can throw away // 
{
	size_t i = 0;
	
	if (size == 0)
		return (ft_strlen(src));
	while (src[i] != '\0' && i < (size - 1))
	{
		dest[i] = src[i];
		i++;
	}
	dest[i] = '\0';
	return (ft_strlen(src));
}

char	**map_to_envp(const std::map<std::string, std::string> &env_map) // this is to create a new envp variable which has your HTTP methods, and all the other variables that are associated //
{
	char	**envp = new char*[env_map.size() + 1]; // hardcoded a env_map variable in main //
	size_t	index = 0;
	
	for (std::map<std::string, std::string>::const_iterator it = env_map.begin();
		it != env_map.end(); ++it) // const iterator so that values doesnt change //
	{
		std::string pair = it->first + "=" + it->second;
		char	*cstr = (char *)malloc(pair.size() + 1);
		if (cstr == NULL)
		{
			for (size_t j = 0; j < index; ++j)
				free(envp[j]);
			delete[] envp;
			return (NULL);
		}
		ft_strlcpy(cstr, pair.c_str(), pair.size() + 1); // convert pair to C string so you can copy //
		envp[index++] = cstr;
	}
	envp[index] = NULL;
	return (envp);
}

void	free_envp(char **envp) // freeing the new envp variable that you have created //
{
	if (envp == NULL)
		return;
	for (size_t i = 0; envp[i] != NULL; ++i)
		free(envp[i]);
	delete[] envp;
}

ssize_t	write_all(int fd, const char *buf, size_t size) // making sure no partial writes Loop it through //
{
	size_t	total_written = 0;
	while (total_written < size)
	{
		ssize_t	written = write(fd, buf + total_written, size - total_written);
		if (written <= 0)
		{
			if (errno == EINTR) // Macro for signal interrupting the write process //
				continue ;
			return (-1);	
		}
		total_written += written; 
	}
	return (total_written); // return total amount of data written //
}


int	main(int ac, char **av, char **envp)
{
	(void) ac;
	(void) av;
	(void) envp;
	int	pipe_stdin[2]; // parent to talk to child //
	int	pipe_stdout[2]; // child to talk to parent //
	
	if (create_pipe(pipe_stdin) == -1 || create_pipe(pipe_stdout) == -1) // create pipes first //
		return (-1);
	std::map<std::string, std::string> env_map; // hardcoding data. Expect this data to be taken from our classes or structure //
	env_map["REQUEST_METHOD"] = "DELETE";
	env_map["QUERY_STRING"] = "name=alice&age=25";
	env_map["CONTENT_LENGTH"] = "20";
	env_map["CONTENT_TYPE"] = "text/html";
	env_map["SCRIPT_NAME"] = "/cgi-bin/test.cgi";
	env_map["SERVER_PORT"] = "8080";
	env_map["SERVER_PROTOCOL"] = "HTTP/1.1";
	env_map["REMOTE_ADDR"] = "127.0.0.1";
	env_map["HTTP_USER_AGENT"] = "Mozilla/5.0";
	env_map["HTTP_COOKIE"] = "session=abc123";
	env_map["HTTP_REFERER"] = "http://localhost/home";
	env_map["GATEWAY_INTERFACE"] = "CGI/1.1";
	
	char	**cgi_envp = map_to_envp(env_map); // form a new environment variable so that we can use execve //
	if (cgi_envp == NULL)
	{
		std::cerr << "Failed to allocate env." << std::endl;
		return (-1);
	}
	// initially i used strings, but i realise i needed map so that i can overwrite data easily //
	/*std::vector<char *> cgi_envp;
	for (size_t i = 0; i < env_vars.size(); ++i)
		cgi_envp.push_back(ft_strdup(env_vars[i].c_str()));
	cgi_envp.push_back(NULL);*/ // this needs to change because i was using strings to capture it // 
	pid_t	pid = fork();
	if (pid  == -1)
	{
		perror("fork failed");
		free_envp(cgi_envp);
		return (-1);
	}
	if (pid == 0) // child process //
	{
		// in the child process, we read from stdin pipe and writes to stdout pipe //
		close(pipe_stdin[1]); // in the child we only read data from stdin, no write end is needed //
		close(pipe_stdout[0]); // in the child, we only need to write data to stdout , no read end is needed //
		
		dup2(pipe_stdin[0], STDIN_FILENO); // use dup2 on the read end of stdin //
		dup2(pipe_stdout[1], STDOUT_FILENO); // use dup2 on the write end of stdout //
		close(pipe_stdin[0]); // close off after successful dup //
		close(pipe_stdout[1]); // close off after successful dup //
		
		// creating paths to do execve //
		const char *cgi_path = "/home/apoh/Documents/test/webserv/cgi-bin/my_cgi.cgi";
		char	*argv[] = {const_cast<char*>(cgi_path), NULL}; // need to cast to this format because execve requires a particular type to run execve //
		
		// might not be important. I was testing something earlier on //
		/*std::vector<std::string> argv_vec;
		argv_vec.push_back("/bin/echo");
		argv_vec.push_back("hello, world");
		argv_vec.push_back("cgi test!");
		
		std::vector<char *> argv;
		for (size_t i = 0; i < argv_vec.size(); ++i)
			argv.push_back(const_cast<char*>(argv_vec[i].c_str()));
		argv.push_back(NULL);*/ // another example//
		execve(cgi_path, argv, cgi_envp);
		perror("execve failed");
		exit(EXIT_FAILURE);
		
		// same as this. might not be important // 
		/*const char *cgi_response = 
			"HTTP/1.1 200 OK\r\n"
			"Content-Type: text/plain\r\n"
			"\r\n"
			"Hello from the CGI child!\n";
		write(STDOUT_FILENO, cgi_response, ft_strlen(cgi_response));*/ // one example //
		/*// read the data from stdin from parent //
		char	buf_child[BUFFER_SIZE];
		ssize_t bytes_read_child = read(STDIN_FILENO, buf_child, BUFFER_SIZE - 1);
		if (bytes_read_child > 0)
		{
			buf_child[bytes_read_child] = '\0';
			write(STDOUT_FILENO, buf_child, bytes_read_child);
		}*/ // another example //
	}
	else // parent //
	{
		// so parent writes to stdin pipe and reads from stdout pipe //
		close(pipe_stdin[0]); // in the parent, we dont need the read end from stdin //  
		close(pipe_stdout[1]); // in the parent, we dont need the write end from stdout //
		
		const char *msg = "Hello from Parent!\n";
		if (write_all(pipe_stdin[1], msg, ft_strlen(msg)) == -1) // if writing failed, we kill off the child process, the pipes, and free off memory // 
		{
			perror("Write to CGI stdin failed");
			kill(pid, SIGKILL);
			close(pipe_stdin[1]);
			close(pipe_stdout[0]);
			free_envp(cgi_envp);
			return (1);
		}
		close(pipe_stdin[1]); // no writing is needed, so close first // 
		
		std::string	cgi_output;
		char	buf_parent[BUFFER_SIZE];
		ssize_t	bytes_read_parent;
		const int	TIMEOUT_SECONDS = 3; // seting up a timer in case your program hangs indefinitely // 
		while (true)
		{
			fd_set	readfds; // using select here but we using epoll so i figuring how it works // 
			FD_ZERO(&readfds); // clears the set // 
			FD_SET(pipe_stdout[0], &readfds); // adds the set to be monitored // 
			
			struct timeval	tv;
			tv.tv_sec = TIMEOUT_SECONDS; // seconds //
			tv.tv_usec = 0; // millisecs //
			
			int	retval = select(pipe_stdout[0] + 1, &readfds, NULL, NULL, &tv);
			if (retval == -1) // if error // 
			{
				perror("select");
				break ;
			}
			else if (retval == 0) // if timeout // 
			{
				std::cerr << "Timeout waiting for CGI output. Killing child process." << std::endl;
				kill(pid, SIGKILL); // kill off child process // 
				break ;
			}
			if (FD_ISSET(pipe_stdout[0], &readfds)) // if pipe is ready to be read //
			{
				bytes_read_parent = read(pipe_stdout[0], buf_parent, BUFFER_SIZE - 1);
				if (bytes_read_parent == -1) // if read fails //
				{
					perror("read");
					break ;
				}
				else if (bytes_read_parent == 0) // if nothing left to read //
				{
					break ; // EOF has reached //
				}
				buf_parent[bytes_read_parent] = '\0'; // once read complete, put a null terminator // 
				cgi_output += buf_parent; // concentate the strings together since i have a vector string // 
			}
			
			std::cout << "Parent received:\n" << cgi_output; // result of output //
			close(pipe_stdout[0]); // now can close stdout pipe //
			
			int	status;
			waitpid(pid, &status, 0);
			
			free_envp(cgi_envp);
		}
	}
	return (0);
}
