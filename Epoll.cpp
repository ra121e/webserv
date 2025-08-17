#include "Epoll.hpp"
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <map>
#include <stdexcept>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/timerfd.h>
#include "Timer.hpp"

Epoll::Epoll(int _fd): BaseFile(_fd), events()
{
}

Epoll::~Epoll()
{
	for (std::map<int, ClientConnection*>::iterator it = clients.begin(); it != clients.end(); ++it)
	{
		delete it->second;
	}
}

bool	Epoll::TimeGreater::operator()(ClientConnection* a, ClientConnection* b) const
{
	return a->getExpiry() > b->getExpiry();
}

void	Epoll::addEventListener(Server* server, int listen_fd)
{
	// set server fd and event into event struct (its like "entry sheet" or "application form")
	struct epoll_event	event = {};
	event.events = EPOLLIN;
	event.data.fd = listen_fd;

	// registration event into epoll
	if (epoll_ctl(getFd(), EPOLL_CTL_ADD, listen_fd, &event) == -1)
	{
		throw std::runtime_error(strerror(errno));
	}
	servers[listen_fd] = server;
}

// set client fd and event to "application form"
void	Epoll::addClient(int server_fd)
{
	Server				*server = servers[server_fd];
	ClientConnection	*client = new ClientConnection(server_fd, server, time(NULL) + TIMEOUT);

	struct epoll_event	event = {};
	event.events = EPOLLIN;
	event.data.fd = client->getFd();

	// registration the client socket into epoll
	if (epoll_ctl(getFd(), EPOLL_CTL_ADD, client->getFd(), &event) == -1)
	{
		throw std::runtime_error(strerror(errno));
	}
	client->retrieveHost();
	clients[client->getFd()] = client;
	expiry_queue.push(client);
}

void Epoll::modifyEventListener(ClientConnection *client) const
{
	// set server fd and event into event struct (its like "entry sheet" or "application form")
	struct epoll_event event = {};
	event.events = EPOLLOUT;
	event.data.fd = client->getFd();

	// registration event into epoll
	if (epoll_ctl(getFd(), EPOLL_CTL_MOD, client->getFd(), &event) == -1)
	{
		throw std::runtime_error(strerror(errno));
	}
}

void	Epoll::handleEvents()
{
	// ask kernel of event happening
	int	num_events = epoll_wait(getFd(), events, MAX_EVENTS, -1);
	if (num_events == -1)
	{
		throw std::runtime_error(strerror(errno));
	}
	// event check
	for (int i = 0; i < num_events; ++i)
	{
		int current_fd = events[i].data.fd;
		std::map<int, Server*>::iterator	it = servers.find(current_fd);
		// new client access event
		if (it != servers.end())
		{
			addClient(current_fd);
		}
		else
		{
			std::map<int, ClientConnection*>::iterator ite = clients.find(current_fd);
			if (ite == clients.end())
			{
				throw std::runtime_error("Unexpected error encountered");
			}
			ClientConnection*	client = ite->second;

			if (events[i].events & (EPOLLERR | EPOLLHUP))
			{
				// Drain the socket
				if (events[i].events & EPOLLIN)
				{
					char	buf[BUFSIZ];
					for (ssize_t bytes_read = read(current_fd, buf, sizeof(buf));
						bytes_read > 0; bytes_read = read(current_fd, buf, sizeof(buf)));
				}
				delete client;
				clients.erase(current_fd);
			}
			else if (events[i].events & EPOLLIN)
			{
				// receive data from client fd
				char	buf[BUFSIZ];
				ssize_t bytes_read = read(current_fd, buf, sizeof(buf));
				if (bytes_read == 0)
				{
					delete client;
					clients.erase(current_fd);
					continue; // Client disconnected
				}
				if (bytes_read == -1)
				{
					client->setServerError(true);
				}
				else
				{
					client->appendToBuffer(buf, static_cast<size_t>(bytes_read));
					if (!client->parseRequest())
					{
						continue; // Not enough data to parse the request
					}
				}
				client->makeResponse();
				modifyEventListener(client);
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

void	Epoll::addTimer()
{
	Timer	timer;

	struct epoll_event event = {};
	event.events = EPOLLIN;
	event.data.fd = timer.getFd();

	if (epoll_ctl(getFd(), EPOLL_CTL_ADD, timer.getFd(), &event) == -1)
	{
		throw std::runtime_error(strerror(errno));
	}
}
