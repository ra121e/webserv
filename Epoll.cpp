#include "Epoll.hpp"
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <map>
#include <stdexcept>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/timerfd.h>
#include <sys/types.h>
#include "ConnectionExpiration.hpp"
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
	for (std::map<int, CGI*>::iterator it = cgis.begin(); it != cgis.end(); ++it)
	{
		delete it->second;
	}
}

// set client fd and event to "application form"
void	Epoll::addClient(int server_fd)
{
	time_t				current_time = time(NULL);
	ClientConnection	*client = new ClientConnection(server_fd, servers[server_fd], current_time + TIMEOUT);

	modifyEpoll(client->getFd(), EPOLLIN, EPOLL_CTL_ADD);
	clients[client->getFd()] = client;
	expiry_queue.push(ConnectionExpiration(client->getFd(), current_time + TIMEOUT));
	timer.setTimer(TIMEOUT);
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
		else if (current_fd == timer.getFd())
		{
			uint64_t expirations = 0;
			read(timer.getFd(), &expirations, sizeof(expirations));

			while (!expiry_queue.empty() && expiry_queue.top().getExpiration() <= time(NULL))
			{
				ConnectionExpiration	exp = expiry_queue.top();
				expiry_queue.pop();
				std::map<int, ClientConnection*>::iterator client_it = clients.find(exp.getFd());
				if (client_it != clients.end())
				{
					ClientConnection* client = client_it->second;
					if (client->getExpiry() != exp.getExpiration())
					{
						continue;
					}
					client->setTimedOut(true);
					client->makeResponse(*this);
					modifyEpoll(client->getFd(), EPOLLOUT, EPOLL_CTL_MOD);
				}
			}
			if (!expiry_queue.empty())
			{
				timer.setTimer(expiry_queue.top().getExpiration() - time(NULL));
			}
		}
		else
		{
			std::map<int, ClientConnection*>::iterator ite = clients.find(current_fd);
			if (ite != clients.end())
			{
				handleClientsAndCgis<ClientConnection>(ite->second, events[i].events, current_fd);
				continue;
			}
			std::map<int, CGI*>::iterator itc = cgis.find(current_fd);
			if (itc != cgis.end())
			{
				handleClientsAndCgis<CGI>(itc->second, events[i].events, current_fd);
				continue;
			}
			throw std::runtime_error("Unknown file descriptor in epoll events");
		}
	}
}

void	Epoll::addTimer()
{
	modifyEpoll(timer.getFd(), EPOLLIN, EPOLL_CTL_ADD);
}

void	Epoll::modifyEpoll(int fd, uint32_t _events, int mode) const
{
	struct epoll_event event = {};
	event.events = _events;
	event.data.fd = fd;

	if (epoll_ctl(getFd(), mode, fd, &event) == -1)
	{
		throw std::runtime_error(strerror(errno));
	}
}
