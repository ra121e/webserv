#ifndef EPOLL_HPP
#define EPOLL_HPP
#include <cstdio>
#include <functional>
#include <map>
#include <queue>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <sys/epoll.h>
#include <netinet/in.h>
#include "BaseFile.hpp"
#include "Timer.hpp"
#include "ConnectionExpiration.hpp"

class Server;
class ClientConnection;
class CGI;

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
	void	addToMap(int _fd, T* resource);
	template<typename T>
	void	removeResource(int resourceFd, T* resource);
	template<typename T>
	void	handleReadError(int resourceFd, T* resource);
	template<typename T>
	void	handleServerWrite(T* resource, int event_fd);
public:
	Epoll(int _fd);
	~Epoll();
	void	addClient(int server_fd);
	void	handleEvents();
	void	addTimer();
	void	modifyEpoll(int _fd, uint32_t _events, int mode) const;
	template<typename T>
	void	addResource(int _fd, T* resource);
	template<typename T>
	void	handleClientsAndCgis(T* resource, uint32_t _events, int event_fd);
	template<typename T>
	bool	handleReadFromClient(T* resource, int event_fd, const char* buf, ssize_t bytes_read);
	template<typename T>
	void	prepWriteToClient(T* resource, const char* buf);
};

#endif

template<typename T>
void	Epoll::addResource(int _fd, T* resource)
{
	modifyEpoll(_fd, EPOLLIN, EPOLL_CTL_ADD);
	addToMap<T>(_fd, resource);
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
			if (!handleReadFromClient<T>(resource, event_fd, buf, bytes_read))
			{
				// Handle case where not enough data was received
				return;
			}
		}
		prepWriteToClient<T>(resource, buf);
	}
	else if (_events & EPOLLOUT)
	{
		handleServerWrite<T>(resource, event_fd);
	}
}
