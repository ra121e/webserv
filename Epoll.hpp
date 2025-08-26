#ifndef EPOLL_HPP
#define EPOLL_HPP
class Epoll;
#include <functional>
#include <map>
#include <queue>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <sys/epoll.h>
#include <netinet/in.h>
#include "ClientConnection.hpp"
#include "Server.hpp"
#include "BaseFile.hpp"
#include "Timer.hpp"
#include "ConnectionExpiration.hpp"
#include "CGI.hpp"

class Epoll: public BaseFile
{
private:
	std::map<int, Server*>	servers;
	std::map<int, ClientConnection*>	clients;
	std::map<int, CGI*>	cgis;
	static const int	MAX_EVENTS = 64;
	struct epoll_event	events[MAX_EVENTS];
	Timer	timer;
	static const time_t	TIMEOUT = 10;
	std::priority_queue<ConnectionExpiration, std::vector<ConnectionExpiration>, std::greater<ConnectionExpiration> >	expiry_queue;

	Epoll(const Epoll& other);
	Epoll&	operator=(const Epoll& other);
	template<typename T>
	void	addToMap(int fd, T* resource);
	template<typename T>
	void	removeResource(int resourceFd, T* resource);
	template<typename T>
	void	handleReadError(int resourceFd, T* resource);
public:
	Epoll(int _fd);
	~Epoll();
	void	addClient(int server_fd);
	void	handleEvents();
	void	addTimer();
	void	modifyEpoll(int fd, uint32_t _events, int mode) const;
	template<typename T>
	void	addResource(int fd, T* resource);
	template<typename T>
	void	handleClientsAndCgis(T* resource, uint32_t _events, int event_fd);
};

#endif

template<typename T>
void	Epoll::addResource(int fd, T* resource)
{
	modifyEpoll(fd, EPOLLIN, EPOLL_CTL_ADD);
	addToMap<T>(fd, resource);
}

template<>
void	Epoll::addToMap(int fd, Server* resource)
{
	servers[fd] = resource;
}

template<>
void	Epoll::addToMap(int fd, CGI* resource)
{
	cgis[fd] = resource;
}

template<typename T>
void	Epoll::handleClientsAndCgis(T* resource, uint32_t _events, int event_fd)
{
	if (_events & (EPOLLERR | EPOLLHUP))
	{
		// Drain the socket
		if (_events & EPOLLIN)
		{
			char	buf[BUFSIZ];
			for (ssize_t bytes_read = read(event_fd, buf, sizeof(buf));
				bytes_read > 0; bytes_read = read(event_fd, buf, sizeof(buf)));
		}
		removeResource<T>(event_fd, resource);
	}
	else if (_events & EPOLLIN)
	{
		// receive data from client fd
		char	buf[BUFSIZ];
		ssize_t bytes_read = read(event_fd, buf, sizeof(buf));
		if (bytes_read == 0)
		{
			removeResource<T>(event_fd, resource);
			return;
		}
		if (bytes_read == -1)
		{
			handleReadError<T>(event_fd, resource);
		}
		else
		{
			time_t current_time = time(NULL);
			expiry_queue.push(ConnectionExpiration(event_fd, current_time + TIMEOUT));
			client->setExpiry(current_time + TIMEOUT);
			timer.setTimer(expiry_queue.top().getExpiration() - current_time);
			client->appendToBuffer(buf, static_cast<size_t>(bytes_read));
			if (!client->parseRequest())
			{
				continue; // Not enough data to parse the request
			}
		}
		client->makeResponse(*this);
		modifyEpoll(client->getFd(), EPOLLOUT, EPOLL_CTL_MOD);
	}
	else if (_events & EPOLLOUT)
	{
		if (client->sendResponse())
		{
			delete client;
			clients.erase(current_fd);
		}
	}
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
	resource->setServerError(true);
}

template<>
void	Epoll::handleReadError(int resourceFd, CGI* resource)
{
	removeResource<CGI>(resourceFd, resource);
	resource->client->setServerError(true);
}
