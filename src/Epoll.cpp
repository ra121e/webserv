#include "../include/Epoll.hpp"
#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <map>
#include <stdexcept>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/timerfd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "../include/ConnectionExpiration.hpp"
#include "../include/Timer.hpp"
#include "../include/ClientConnection.hpp"
#include "../include/CGI.hpp"
#include "../include/Server.hpp"

Epoll::Epoll(int _fd): BaseFile(_fd), events()
{
	modifyEpoll(request_timer.getFd(), EPOLLIN, EPOLL_CTL_ADD);
	modifyEpoll(cgi_timer.getFd(), EPOLLIN, EPOLL_CTL_ADD);
}

Epoll::~Epoll()
{
	for (std::map<int, ClientConnection*>::iterator it = clients.begin(); it != clients.end(); ++it)
	{
		delete it->second;
	}
	for (std::map<int, CGI*>::iterator it = server_pipe_read_fds.begin(); it != server_pipe_read_fds.end(); ++it)
	{
		delete it->second;
	}
}

// set client fd and event to "application form"
void	Epoll::addClient(int server_fd)
{
	time_t				current_time = time(NULL);
	ClientConnection	*client = new ClientConnection(server_fd, servers[server_fd], current_time + REQUEST_TIMEOUT);

	modifyEpoll(client->getFd(), EPOLLIN, EPOLL_CTL_ADD);
	clients[client->getFd()] = client;
	expiry_queue.push(ConnectionExpiration(client->getFd(), current_time + REQUEST_TIMEOUT));
	request_timer.setTimer(REQUEST_TIMEOUT);
}

void	Epoll::addServer(int _fd, Server* server)
{
	servers[_fd] = server;
	modifyEpoll(_fd, EPOLLIN, EPOLL_CTL_ADD);
}

void	Epoll::addPipeFds(CGI* cgi)
{
	server_pipe_read_fds[cgi->get_server_read_fd()] = cgi;
	server_pipe_write_fds[cgi->get_server_write_fd()] = cgi;
	modifyEpoll(cgi->get_server_read_fd(), EPOLLIN, EPOLL_CTL_ADD);
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
		else if (current_fd == request_timer.getFd())
		{
			handleRequestTimeOut();
		}
		else if (current_fd == cgi_timer.getFd())
		{
			handleCgiTimeOut();
		}
		else
		{
			std::map<int, ClientConnection*>::iterator ite = clients.find(current_fd);
			if (ite != clients.end())
			{
				handleClientsAndCgis<ClientConnection>(ite->second, events[i].events, current_fd);
				continue;
			}
			std::map<int, CGI*>::iterator itc = server_pipe_read_fds.find(current_fd);
			if (itc != server_pipe_read_fds.end())
			{
				handleClientsAndCgis<CGI>(itc->second, events[i].events, current_fd);
				continue;
			}
			std::map<int, CGI*>::iterator itw = server_pipe_write_fds.find(current_fd);
			if (itw != server_pipe_write_fds.end())
			{
				handleClientsAndCgis<CGI>(itw->second, events[i].events, current_fd);
				continue;
			}
		}
	}
}

void	Epoll::modifyEpoll(int _fd, uint32_t _events, int mode) const
{
	struct epoll_event event = {};
	event.events = _events;
	event.data.fd = _fd;

	if (epoll_ctl(getFd(), mode, _fd, &event) == -1)
	{
		throw std::runtime_error("epoll_ctl: " + std::string(strerror(errno)));
	}
}

template<>
void	Epoll::addFdToEpoll(int _fd, Server* resource)
{
	servers[_fd] = resource;
}

template<>
void	Epoll::addFdToEpoll(int _fd, CGI* resource)
{
	server_pipe_read_fds[_fd] = resource;
}

template<>
void	Epoll::removeResource(int _fd, ClientConnection* resource)
{
	clients.erase(_fd);
	delete resource;
}

template<>
void	Epoll::removeResource(int _fd, CGI* resource)
{
	server_pipe_read_fds.erase(_fd);
	delete resource;
}

template<>
void	Epoll::handleReadError(int /*unused*/, ClientConnection* resource)
{
	resource->setServerError(true);
}

template<>
void	Epoll::handleReadError(int resourceFd, CGI* resource)
{
	removeResource<CGI>(resourceFd, resource);
	resource->get_client()->setServerError(true);
}

template<>
bool	Epoll::handleReadFromResource(ClientConnection* resource, int event_fd, const char* buf, ssize_t bytes_read)
{
	time_t current_time = time(NULL);
	expiry_queue.push(ConnectionExpiration(event_fd, current_time + REQUEST_TIMEOUT));
	resource->setExpiration(current_time + REQUEST_TIMEOUT);
	request_timer.setTimer(expiry_queue.top().getExpiration() - current_time);
	resource->appendToBuffer(buf, static_cast<size_t>(bytes_read));
	return resource->parseRequest();
}

template<>
bool	Epoll::handleReadFromResource(CGI* resource, int /*unused*/, const char* buf, ssize_t bytes_read)
{
	resource->get_client()->appendToResBuffer(buf, static_cast<size_t>(bytes_read));
	while (waitpid(-1, NULL, 0) > 0){}
	return true;
}

template<>
void	Epoll::prepRequestFrom(ClientConnection* resource)
{
	resource->makeResponse(*this);
	if (resource->getRequest().forward_to_cgi)
	{
		// If the request was forwarded to CGI, we don't need to send a response here
		return;
	}
	modifyEpoll(resource->getFd(), EPOLLOUT, EPOLL_CTL_MOD);
}

template<>
void	Epoll::prepRequestFrom(CGI* resource)
{
	modifyEpoll(resource->get_client()->getFd(), EPOLLOUT, EPOLL_CTL_MOD);
	server_pipe_read_fds.erase(resource->get_server_read_fd());
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

template<>
void	Epoll::handleServerWrite(CGI* resource, int event_fd)
{
	const std::string& input = resource->get_client()->getBuffer();
	write(event_fd, input.c_str(), input.size());
	modifyEpoll(event_fd, 0, EPOLL_CTL_DEL);
	resource->close_server_write_fd();
	server_pipe_write_fds.erase(event_fd);
}

void	Epoll::handleRequestTimeOut()
{
	uint64_t expirations = 0;
	read(request_timer.getFd(), &expirations, sizeof(expirations));

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
		request_timer.setTimer(expiry_queue.top().getExpiration() - time(NULL));
	}
}

void	Epoll::handleCgiTimeOut()
{
	uint64_t expirations = 0;
	read(cgi_timer.getFd(), &expirations, sizeof(expirations));

	while (!cgi_expiry_map.empty() && cgi_expiry_map.begin()->first <= time(NULL))
	{
		CGI*	cgi = cgi_expiry_map.begin()->second;
		cgi_expiry_map.erase(cgi_expiry_map.begin());
		std::map<int, ClientConnection*>::iterator client_it = clients.find(cgi->get_client()->getFd());
		if (client_it != clients.end())
		{
			kill(cgi->getPid(), SIGKILL);
			waitpid(cgi->getPid(), NULL, 0);
			ClientConnection* client = client_it->second;
			client->setServerError(true);
			client->makeResponse(*this);
			modifyEpoll(client->getFd(), EPOLLOUT, EPOLL_CTL_MOD);
		}
		delete cgi;
	}
	if (!cgi_expiry_map.empty())
	{
		cgi_timer.setTimer(cgi_expiry_map.begin()->first - time(NULL));
	}
}

void	Epoll::addCgiExpiry(CGI* cgi)
{
	time_t	now = time(NULL);
	cgi_expiry_map[now + CGI_TIMEOUT] = cgi;
	cgi_timer.setTimer(cgi_expiry_map.begin()->first - now);
}
