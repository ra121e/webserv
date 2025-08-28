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
#include "ClientConnection.hpp"
#include "CGI.hpp"
#include "Server.hpp"

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
					if (client->getExpiration() != exp.getExpiration())
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

void	Epoll::modifyEpoll(int _fd, uint32_t _events, int mode) const
{
	struct epoll_event event = {};
	event.events = _events;
	event.data.fd = _fd;

	if (epoll_ctl(getFd(), mode, _fd, &event) == -1)
	{
		throw std::runtime_error(strerror(errno));
	}
}

template<>
void	Epoll::addToMap(int _fd, Server* resource)
{
	servers[_fd] = resource;
}

template<>
void	Epoll::addToMap(int _fd, CGI* resource)
{
	cgis[_fd] = resource;
}

template<>
void	Epoll::removeResource(int resourceFd, ClientConnection* resource)
{
	clients.erase(resourceFd);
	delete resource;
}

template<>
void	Epoll::removeResource(int resourceFd, CGI* resource)
{
	cgis.erase(resourceFd);
	delete resource;
}

template<>
void	Epoll::handleReadError(int resourceFd, ClientConnection* resource)
{
	(void)resourceFd; // Unused parameter
	resource->setServerError(true);
}

template<>
void	Epoll::handleReadError(int resourceFd, CGI* resource)
{
	removeResource<CGI>(resourceFd, resource);
	resource->get_client()->setServerError(true);
}

template<>
bool	Epoll::handleReadFromClient(ClientConnection* resource, int event_fd, const char* buf, ssize_t bytes_read)
{
	time_t current_time = time(NULL);
	expiry_queue.push(ConnectionExpiration(event_fd, current_time + TIMEOUT));
	resource->setExpiration(current_time + TIMEOUT);
	timer.setTimer(expiry_queue.top().getExpiration() - current_time);
	resource->appendToBuffer(buf, static_cast<size_t>(bytes_read));
	return resource->parseRequest();
}

template<>
bool	Epoll::handleReadFromClient(CGI* resource, int event_fd, const char* buf, ssize_t bytes_read)
{
	(void)resource;
	(void)event_fd;
	(void)buf;
	(void)bytes_read;
	delete resource; // For CGI, we don't need to keep reading from the pipe after the initial read
	cgis.erase(event_fd);
	return true; // Always return true for CGI as we don't parse requests here
}

template<>
void	Epoll::prepWriteToClient(ClientConnection* resource, const char* buf)
{
	(void)buf; // Unused parameter
	resource->makeResponse(*this);
	modifyEpoll(resource->getFd(), EPOLLOUT, EPOLL_CTL_MOD);
}

template<>
void	Epoll::prepWriteToClient(CGI* resource, const char* buf)
{
	resource->writeToInputBuffer(buf);
	modifyEpoll(resource->get_client()->getFd(), EPOLLOUT, EPOLL_CTL_MOD);
}

template<>
void	Epoll::handleServerWrite(CGI* resource, int event_fd)
{
	write(event_fd, resource->getInputBuffer().c_str(), resource->getInputBuffer().size());
}

template<>
void	Epoll::handleServerWrite(ClientConnection* resource, int event_fd)
{
	if (resource->sendResponse())
	{
		clients.erase(event_fd);
		delete resource;
	}
}
