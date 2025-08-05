/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgoh <cgoh@student.42singapore.sg>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/09 13:04:22 by athonda           #+#    #+#             */
/*   Updated: 2025/08/05 19:47:17 by cgoh             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "main.hpp"
#include <cerrno>
#include <exception>
#include <stdexcept>
#include <cstring>
#include <sys/epoll.h>
#include <vector>

int	main(int argc, char **argv)
{
	int	client_fd;
	char	buf[1024];
	Config	conf;
	static const int	MAX_EVENTS = 64;
	struct epoll_event	events[MAX_EVENTS];

	if (argc != 2)
	{
		std::cerr << "Usage: ./webserv [configuration file]\n";
		return 1;
	}
	try
	{
		get_file_config(argv[1], conf);
		conf.setupServers();
		Epoll	epoll(epoll_create1(EPOLL_CLOEXEC));
		for (std::vector<Server>::const_iterator s_it = conf.getServers().begin(); s_it != conf.getServers().end(); ++s_it)
		{
			for (std::vector<Network>::const_iterator n_it = s_it->getNetworks().begin(); n_it != s_it->getNetworks().end(); ++n_it)
			{
				epoll.addEventListener(n_it->getFd());
			}
		}
		while (true)
		{
			// ask kernel of event happening
			int	num_events = epoll_wait(epoll.getFd(), events, MAX_EVENTS, -1);
			if (num_events == -1)
			{
				throw std::runtime_error(strerror(errno));
			}

			// event check
			for (size_t i = 0; i < static_cast<size_t>(num_events); ++i)
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
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << "\n";
		return 1;
	}
}
