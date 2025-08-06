#include "Epoll.hpp"
#include <algorithm>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <exception>
#include <memory>
#include <stdexcept>
#include <vector>
#include <fcntl.h>

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
	for (std::vector<Client*>::iterator it = clients.begin(); it != clients.end(); ++it)
	{
		delete *it;
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
void	Epoll::addClient(Client* client)
{
	struct epoll_event	event = {};
	event.events = EPOLLIN | EPOLLET;
	event.data.fd = client->getFd();

	// registration the client socket into epoll
	if (epoll_ctl(fd, EPOLL_CTL_ADD, client->getFd(), &event) == -1)
	{
		delete client;
		throw std::runtime_error(strerror(errno));
	}
	clients.push_back(client);
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
				struct sockaddr_in	client_addr = {};
				socklen_t	addrlen = sizeof(client_addr);
				try
				{
					Client*	client(new Client(accept(current_fd, reinterpret_cast<struct sockaddr*>(&client_addr), &addrlen)));
					client->setNonBlocking();
					addClient(client);
				}
				catch (const std::invalid_argument&)
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
					Client*	clientToDelete = NULL;

					for (std::vector<Client*>::iterator ite = clients.begin(); ite != clients.end(); ++ite)
					{
						if ((*ite)->getFd() == current_fd)
						{
							clientToDelete = *ite;
							break;
						}
					}
					clients.erase(std::remove(clients.begin(), clients.end(), clientToDelete), clients.end());
					break ;
				}
				/*
				Do something with the data read
				*/
			}
		}
	}
}
