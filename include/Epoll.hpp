#ifndef EPOLL_HPP
#define EPOLL_HPP
#include <cstdio>
#include <ctime>
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
#include "CgiExpiration.hpp"

class Server;
class ClientConnection;
class CGI;

class Epoll: public BaseFile
{
private:
	std::map<int, SharedPointer<Server> >	servers;
	std::map<int, SharedPointer<ClientConnection> >	clients;
	std::map<int, SharedPointer<CGI> >	server_pipe_read_fds;
	std::map<int, SharedPointer<CGI> >	server_pipe_write_fds;
	static const int	MAX_EVENTS = 64;
	struct epoll_event	events[MAX_EVENTS];
	Timer	request_timer;
	Timer	cgi_timer;
	static const time_t	REQUEST_TIMEOUT = 10;
	static const time_t	CGI_TIMEOUT = 3;
	std::priority_queue<ConnectionExpiration, std::vector<ConnectionExpiration>,
		std::greater<ConnectionExpiration> >	expiry_queue;
	std::priority_queue<CGIExpiration, std::vector<CGIExpiration>,
		std::greater<CGIExpiration> >	cgi_expiry_queue;

	static void	handleReadError(int, const SharedPointer<ClientConnection>& client);
	void	handleReadError(int resourceFd, const SharedPointer<CGI>& cgi);
	void	handleServerWrite(const SharedPointer<ClientConnection>& client, int event_fd);
	void	handleServerWrite(const SharedPointer<CGI>& cgi, int event_fd);
	void	addFdToEpoll(int _fd, const SharedPointer<CGI>& cgi);
	void	addFdToEpoll(int _fd, const SharedPointer<Server>& server);
	void	handleRequestTimeOut();
	void	handleCgiTimeOut();
public:
	Epoll(int _fd);
	void	addClient(int server_fd);
	void	addServer(int _fd, const SharedPointer<Server>& server);
	void	handleEvents();
	void	modifyEpoll(int _fd, uint32_t _events, int mode) const;
	void	addPipeFds(const SharedPointer<CGI>& cgi);
	void	removeResource(int _fd, const SharedPointer<CGI>& cgi);
	void	removeResource(int _fd, const SharedPointer<ClientConnection>& client);
	template<typename T>
	void	handleClientsAndCgis(const T& resource, uint32_t _events, int event_fd);
	bool	handleReadFromResource(const SharedPointer<ClientConnection>& resource, int event_fd,
		const char* buf, ssize_t bytes_read);
	static bool	handleReadFromResource(const SharedPointer<CGI>& cgi, int,
		const char* buf, ssize_t bytes_read);
	void	prepRequestFrom(const SharedPointer<ClientConnection>& client);
	void	prepRequestFrom(const SharedPointer<CGI>& cgi);
	void	addCgiExpiry(const SharedPointer<CGI>& cgi);
};

#endif

template<typename T>
void	Epoll::handleClientsAndCgis(const T& resource, uint32_t _events, int event_fd)
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
		removeResource(event_fd, resource);
	}
	else if (_events & EPOLLIN)
	{
		// receive data from client fd
		char	buf[BUFSIZ];
		ssize_t bytes_read = read(event_fd, buf, sizeof(buf));
		if (bytes_read == 0)
		{
			removeResource(event_fd, resource);
			return;
		}
		if (bytes_read == -1)
		{
			handleReadError(event_fd, resource);
		}
		else
		{
			if (!handleReadFromResource(resource, event_fd, buf, bytes_read))
			{
				// Handle case where not enough data was received
				return;
			}
		}
		prepRequestFrom(resource);
	}
	else if (_events & EPOLLOUT)
	{
		handleServerWrite(resource, event_fd);
	}
}
