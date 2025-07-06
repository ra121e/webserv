/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgoh <cgoh@student.42singapore.sg>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/09 13:04:22 by athonda           #+#    #+#             */
/*   Updated: 2025/07/06 21:33:27 by cgoh             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

// void	printBanner(std::string const &title)
// {
// 	std::cout << "\n--- " << title << " ---" << std::endl;
// }

static bool	read_file(const char *filename)
{
	std::ifstream	infile(filename);
	
	if (!infile)
	{
		std::cerr << "Error: Unable to open " << filename << "\n";
		return false;
	}
	for (std::string line; std::getline(infile, line);)
	{
		std::istringstream	iss(line);
		std::string			word;
		
		while (iss >> word)
		{
			std::cout << word;
		}
		std::cout << "\n";	
	}
	return true;
}

int	main(int argc, char **argv)
{
	int	server_fd;
	int	client_fd;
	struct sockaddr_in	serv_addr;
//	struct sockaddr_in	client_addr;
//	socklen_t	addrlen = sizeof(client_addr);
	char	buf[1024];

	if (argc != 2)
	{
		std::cerr << "Usage: ./webserv [configuration file]\n";
		return 1;
	}
	if (!read_file(argv[1]))
		return 1;
	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd < 0)
		perror("socket");
	std::cout << "socket return value is " << server_fd << std::endl;

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(12345);
	serv_addr.sin_addr.s_addr = INADDR_ANY;

	bind(server_fd, reinterpret_cast<struct sockaddr*>(&serv_addr), sizeof(serv_addr));
	listen(server_fd, 5);

	// set one each poll target
	struct pollfd	poll_target_server;
	poll_target_server.fd = server_fd;
	poll_target_server.events = POLLIN;

	// set series of poll target in vector
	std::vector<struct pollfd> fds;
	fds.push_back(poll_target_server);

	printf("Waiting for connection...\n");



//	client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addrlen);
//	printf("Connected from client\n");

	bool had_client = false;
	while (1)
	{
		int ret = poll(fds.data(), fds.size(), -1);
		if (ret < 0)
			break ;
		for (size_t i = 0; i < fds.size(); ++i)
		{
			// check return envent counter and POLLIN counter "0x0001"
			if (fds[i].revents & POLLIN)
			{
				// new connection from new client
				if (fds[i].fd == server_fd)
				{
					struct sockaddr_in	client_addr;
					socklen_t	addrlen = sizeof(client_addr);
					client_fd = accept(server_fd, reinterpret_cast<struct sockaddr*>(&client_addr), &addrlen);

					std::cout << "new client connected: fd " << client_fd << std::endl;
					struct pollfd	poll_target_client;
					poll_target_client.fd = client_fd;
					poll_target_client.events = POLLIN;
					fds.push_back(poll_target_client);
					had_client = true;
				}
				else
				{
					// receive data from client
					ssize_t n = read(fds[i].fd, buf, sizeof(buf) - 1);
					if (n <= 0)
					{
						close(fds[i].fd);
						fds.erase(fds.begin() + static_cast<std::vector<pollfd>::difference_type>(i));
						--i;
						continue ;
					}
					buf[n] = '\0';
					std::cout << "FD " << fds[i].fd << ">" << buf << std::endl;
					if (!strcmp(buf, "EOM"))
					{
						close(fds[i].fd);
						fds.erase(fds.begin() + static_cast<std::vector<pollfd>::difference_type>(i));
						--i;
					}
				}
			}
		}
		if (had_client && fds.size() == 1)
			break ;
	}
	for (size_t i = 0; i < fds.size(); ++i)
		close (fds[i].fd);
//	close(client_fd);
//	close(server_fd);

//	printBanner("test: write() send message to socket");
//	write(3, "a", 1);
//	close(sock);
	return (0);
}
