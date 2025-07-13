/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgoh <cgoh@student.42singapore.sg>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/09 13:04:22 by athonda           #+#    #+#             */
/*   Updated: 2025/07/13 22:18:47 by cgoh             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "main.hpp"
#include <string.h>

int	main(int argc, char **argv)
{
	int	server_fd;
	int	client_fd;
	struct sockaddr_in	serv_addr;
	char	buf[1024];
	Config	conf;

	if (argc != 2)
	{
		std::cerr << "Usage: ./webserv [configuration file]\n";
		return 1;
	}
	try
	{
		get_file_config(argv[1], conf);
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << "\n";
		return 1;
	}

	// preparation of server socket
	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd < 0)
	{
		perror("socket");
		return (1);
	}
	std::cout << "socket return value is " << server_fd << std::endl;

	// change server socket to non-blocking mode
	if (fcntl(server_fd, F_SETFL, O_NONBLOCK) == -1)
	{
		perror("fcntl NONBLOCK");
		close(server_fd);
		return (1);
	}

	// setting server sockaddr_in struct
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(12345);
	serv_addr.sin_addr.s_addr = INADDR_ANY;

	// bind the socket to port number
	if (bind(server_fd, reinterpret_cast<struct sockaddr*>(&serv_addr), sizeof(serv_addr)) < 0)
	{
		perror("bind");
		close (server_fd);
		return (1);
	}

	if (listen(server_fd, 5) < 0)
	{
		perror("listen");
		close (server_fd);
		return (1);
	}

	printf("Waiting for connection...\n");

	// epoll instance
	int	epoll_fd = epoll_create1(0);
	if (epoll_fd == -1)
	{
		perror("epoll_create1");
		close(server_fd);
		return (1);
	}

	// set server fd and event into event struct (its like "entry sheet" or "application form")
	struct epoll_event	event;
	event.events = EPOLLIN | EPOLLET;
	event.data.fd = server_fd;

	// rigistration event into epoll
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &event) == -1)
	{
		perror("epoll_ctl");
		close(server_fd);
		close(epoll_fd);
		return (1);
	}
	
	static const int	MAX_EVENTS = 64;
	struct epoll_event	events[MAX_EVENTS];

	while (1)
	{
		// ask kernel of event happening
		int	num_events = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
		if (num_events == -1)
		{
			perror("epoll_wait");
			break ;
		}

		// event check
		for (size_t i = 0; i < num_events; ++i)
		{
			int current_fd = events[i].data.fd;

			// new client access event
			if (current_fd == server_fd)
			{
				struct sockaddr_in	client_addr;
				socklen_t	addrlen = sizeof(client_addr);

				// generate new fd for all of queing new clients
				while (1)
				{
					client_fd = accept(server_fd, reinterpret_cast<struct sockaddr*>(&client_addr), &addrlen);
					if (client_fd == -1)
					{
						if (errno == EAGAIN || errno == EWOULDBLOCK)
							break ;
						else
						{
							perror("accept");
							break ;
						}
					}
					std::cout << "new client connected: fd " << client_fd << std::endl;

					// setting the new client socket as previous
					// change client socket to non-blocking mode
					if (fcntl(client_fd, F_SETFL, O_NONBLOCK) == -1)
					{
						perror("fcntl NONBLOCK");
						close(client_fd);
						continue ;
					}

					// set client fd and event to "application form"
					struct epoll_event	event;
					event.events = EPOLLIN | EPOLLET;
					event.data.fd = client_fd;

					// registration the client socket into epoll
					if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event) == -1)
					{
						perror("epoll_ctl");
						close(client_fd);
						continue ;
					}
				}
			}
			// present client request event
			else
			{
				// check if the event is "must read"
				if (events[i].events & POLLIN)
				{
					// read fd untill end
					bool	connection_closed = false;
					while (1)
					{
						// receive data from client fd
						ssize_t n = read(current_fd, buf, sizeof(buf) - 1);
						if (n == -1)
						{
							// nothing to read now, keep connection, I will be back...
							if (errno == EAGAIN || errno == EWOULDBLOCK)
								break ;
							// something error happens
							else
							{
								perror("read");
								connection_closed = true;
								break ;
							}
						}
						// read complete
						else if (n == 0)
						{
							connection_closed = true;
							break ;
						}
						buf[n] = '\0';
						std::cout << "FD " << current_fd << ">" << buf << std::endl;
						if (!strcmp(buf, "EOM"))
						{
							connection_closed = true;
							break ;
						}
					}
					// close fd and withdraw fd from epoll list after disconnection
					if (connection_closed)
					{
						std::cout << "Client disconnected: FD " << current_fd << std::endl;
						if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, current_fd, NULL) == -1)
							perror("epoll_ctl");
						close (current_fd);
					}
				}
			}
		}
	}
	return (0);
}
