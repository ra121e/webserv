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

void	Epoll::addEventListener(Server* server, int listen_fd)
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
	servers[listen_fd] = server;
}

// set client fd and event to "application form"
void	Epoll::addClient(int server_fd)
{
	ClientConnection*	client = new ClientConnection(server_fd);

	struct epoll_event	event = {};
	event.events = EPOLLIN | EPOLLET;
	event.data.fd = client->getFd();

	// registration the client socket into epoll
	if (epoll_ctl(fd, EPOLL_CTL_ADD, client->getFd(), &event) == -1)
	{
		throw std::runtime_error(strerror(errno));
	}
	client->retrieveHost();
	clients[client->getFd()] = client;
}

void Epoll::modifyEventListener(ClientConnection *client)
{
	// set server fd and event into event struct (its like "entry sheet" or "application form")
	struct epoll_event event = {};
	event.events = EPOLLIN | EPOLLOUT | EPOLLET;
	event.data.fd = client->getFd();

	// registration event into epoll
	if (epoll_ctl(fd, EPOLL_CTL_MOD, client->getFd(), &event) == -1)
	{
		throw std::runtime_error(strerror(errno));
	}
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
		std::map<int, Server*>::iterator	it = servers.find(current_fd);
		// new client access event
		if (it != servers.end())
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
					if (errno == EAGAIN)
					{
						break;
					}
					throw;
				}
			}
		}
		else
		{
			std::map<int, ClientConnection*>::iterator ite = clients.find(current_fd);
			if (ite == clients.end())
			{
				throw std::runtime_error("Unexpected error encountered");
			}
			ClientConnection*	client = ite->second;

			if (events[i].events & EPOLLIN)
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
						if (errno == EAGAIN)
						{
							break;
						}
						// something error happens
						throw std::runtime_error(strerror(errno));
					}
					// read complete
					if (bytes_read == 0)
					{
						delete client;
						clients.erase(current_fd);
						break ;
					}
					client->appendToBuffer(buf, static_cast<size_t>(bytes_read));
					// std::cout << "buffer: " << client->getBuffer() << std::endl;

					if (client->parseRequest())
					{
						std::cout << client->getRequest().method << std::endl;
						std::cout << client->getRequest().uri << std::endl;
						std::cout << client->getRequest().version << std::endl;
						std::map<std::string, std::string>::const_iterator it_tmp = client->getRequest().headers.begin();
						for (; it_tmp != client->getRequest().headers.end(); ++it_tmp)
							std::cout << it_tmp->first << ": " << it_tmp->second << std::endl;

						client->makeResponse();
						std::cout << "Response Ready! to FD: " << client->getFd() << std::endl;
						std::cout << client->getResponseBuffer() << std::endl;

						modifyEventListener(client);
					}
				}
			}
			else if (events[i].events & EPOLLOUT)
			{
				if (client->sendResponse())
				{
					delete client;
					clients.erase(current_fd);
				}
			}
		}
	}
}
