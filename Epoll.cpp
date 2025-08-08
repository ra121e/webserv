#include "Epoll.hpp"
#include <algorithm>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <exception>
#include <map>
#include <memory>
#include <stdexcept>
#include <vector>
#include <fcntl.h>
#include <iostream>

Epoll::Epoll(int _fd): fd(_fd)
{
	if (fd == -1)
	{
		throw std::runtime_error(strerror(errno));
	}
}

Epoll::~Epoll()
{
	close(fd);
	for (std::map<int, ClientConnection*>::iterator it = clients.begin(); it != clients.end(); ++it)
	{
		delete it->second;
	}
}

int	Epoll::getFd() const
{
	return fd;
}

void	Epoll::addEventListener(int listen_fd)
{
	// set server fd and event into event struct (its like "entry sheet" or "application form")
	struct epoll_event	event = {};
	event.events = EPOLLIN | EPOLLET;
	event.data.fd = listen_fd;

	// registration event into epoll
	if (epoll_ctl(fd, EPOLL_CTL_ADD, listen_fd, &event) == -1)
	{
		throw std::runtime_error(strerror(errno));
	}
	server_fds.push_back(listen_fd);
}

// set client fd and event to "application form"
void	Epoll::addClient(int server_fd)
{
	struct sockaddr_in	client_addr = {};
	socklen_t	addrlen = sizeof(client_addr);
	int	client_fd = accept(server_fd, reinterpret_cast<struct sockaddr*>(&client_addr), &addrlen);

	if (client_fd < 0)
	{
		throw std::runtime_error(strerror(errno));
	}
	// setting the new client socket as previous
	// change client socket to non-blocking mode
	if (fcntl(client_fd, F_SETFL, O_NONBLOCK) == -1)
	{
		throw std::runtime_error(strerror(errno));
	}

	struct epoll_event	event = {};
	event.events = EPOLLIN | EPOLLET;
	event.data.fd = client_fd;

	// registration the client socket into epoll
	if (epoll_ctl(fd, EPOLL_CTL_ADD, client_fd, &event) == -1)
	{
		throw std::runtime_error(strerror(errno));
	}
	clients[client_fd] = new ClientConnection(client_fd);
}

void	Epoll::handleEvents()
{
	// ask kernel of event happening
	int	num_events = epoll_wait(fd, events, MAX_EVENTS, -1);
	if (num_events == -1)
	{
		throw std::runtime_error(strerror(errno));
	}
	// event check
	for (size_t i = 0; i < static_cast<size_t>(num_events); ++i)
	{
		int current_fd = events[i].data.fd;
		std::vector<int>::iterator	it = std::find(server_fds.begin(), server_fds.end(), current_fd);
		// new client access event
		if (it != server_fds.end())
		{
			// generate new fd for all of queing new clients
			while (true)
			{
				try
				{
					addClient(current_fd);
				}
				catch (const std::exception&)
				{
					if (errno == EAGAIN || errno == EWOULDBLOCK)
					{
						break;
					}
					throw;
				}
			}
		}
		// present client request event
		else if (events[i].events & EPOLLIN)
		{
			std::map<int, ClientConnection*>::iterator ite = clients.find(current_fd);
			if (ite == clients.end())
				throw std::runtime_error("Unexpected error encountered");
			ClientConnection*	ccon = ite->second;
			// read fd untill end
			while (true)
			{
				// receive data from client fd
				char	buf[BUFSIZ];
				ssize_t bytes_read = read(current_fd, buf, sizeof(buf));
				if (bytes_read == -1)
				{
					// nothing to read now, keep connection, I will be back...
					if (errno == EAGAIN || errno == EWOULDBLOCK)
					{
						break;
					}
					// something error happens
					throw std::runtime_error(strerror(errno));
				}
				// read complete
				if (bytes_read == 0)
				{
					delete ccon;
					clients.erase(current_fd);
					break ;
				}
				ccon->append_to_buffer(buf, static_cast<size_t>(bytes_read));
				// std::cout << "buffer: " << ccon->getBuffer() << std::endl;

				if (ccon->parse_request())
				{
					std::cout << ccon->getRequest().method << std::endl;
					std::cout << ccon->getRequest().uri << std::endl;
					std::cout << ccon->getRequest().version << std::endl;
					std::map<std::string, std::string>::const_iterator it_tmp = ccon->getRequest().headers.begin();
					for (; it_tmp != ccon->getRequest().headers.end(); ++it_tmp)
						std::cout << it_tmp->first << ": " << it_tmp->second << std::endl;

					ccon->makeResponse();
					std::cout << "Response Ready! to FD: " << ccon->getFd() << std::endl;
					std::cout << ccon->getResponseBuffer() << std::endl;
				}
			}
		}
	}
}
