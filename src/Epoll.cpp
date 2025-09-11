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

void	Epoll::addFdToEpoll(int _fd, Server* server)
{
	servers[_fd] = server;
}

void	Epoll::addFdToEpoll(int _fd, CGI* cgi)
{
	server_pipe_read_fds[_fd] = cgi;
}

void	Epoll::removeResource(int _fd, ClientConnection* client)
{
	clients.erase(_fd);
	delete client;
}

void	Epoll::removeResource(int _fd, CGI* cgi)
{
	server_pipe_read_fds.erase(_fd);
	delete cgi;
}

void	Epoll::handleReadError(int /*unused*/, ClientConnection* client)
{
	client->setServerError(true);
}

void	Epoll::handleReadError(int resourceFd, CGI* cgi)
{
	removeResource(resourceFd, cgi);
	cgi->set_client_server_error(true);
}

bool	Epoll::handleReadFromResource(ClientConnection* resource, int event_fd, const char* buf, ssize_t bytes_read)
{
	time_t current_time = time(NULL);
	expiry_queue.push(ConnectionExpiration(event_fd, current_time + REQUEST_TIMEOUT));
	resource->setExpiration(current_time + REQUEST_TIMEOUT);
	request_timer.setTimer(expiry_queue.top().getExpiration() - current_time);
	resource->appendToBuffer(buf, static_cast<size_t>(bytes_read));
	return resource->parseRequest();
}

bool	Epoll::handleReadFromResource(CGI* cgi, int /*unused*/, const char* buf, ssize_t bytes_read)
{
	cgi->append_to_client_res_buffer(buf, static_cast<size_t>(bytes_read));
	waitpid(cgi->getPid(), NULL, 0);
	cgi->setFinished(true);
	return true;
}

void	Epoll::prepRequestFrom(ClientConnection* client)
{
	client->makeResponse(*this);
	if (client->getRequest().forward_to_cgi)
	{
		// If the request was forwarded to CGI, we don't need to send a response here
		return;
	}
	modifyEpoll(client->getFd(), EPOLLOUT, EPOLL_CTL_MOD);
}

void	Epoll::prepRequestFrom(CGI* cgi)
{
	modifyEpoll(cgi->get_client_fd(), EPOLLOUT, EPOLL_CTL_MOD);
	server_pipe_read_fds.erase(cgi->get_server_read_fd());
}

void	Epoll::handleServerWrite(ClientConnection* client, int event_fd)
{
	if (client->sendResponse())
	{
		clients.erase(event_fd);
		delete client;
	}
}

void	Epoll::handleServerWrite(CGI* cgi, int event_fd)
{
	const std::string& input = cgi->get_client_buffer();
	write(event_fd, input.c_str(), input.size());
	modifyEpoll(event_fd, 0, EPOLL_CTL_DEL);
	cgi->close_server_write_fd();
	server_pipe_write_fds.erase(event_fd);
}

void	Epoll::handleRequestTimeOut()
{
	uint64_t expirations = 0;
	read(request_timer.getFd(), &expirations, sizeof(expirations));
	time_t	now = time(NULL);

	while (!expiry_queue.empty() && expiry_queue.top().getExpiration() <= now)
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
		request_timer.setTimer(expiry_queue.top().getExpiration() - now);
	}
}

void	Epoll::handleCgiTimeOut()
{
	uint64_t expirations = 0;
	read(cgi_timer.getFd(), &expirations, sizeof(expirations));
	time_t	now = time(NULL);

	while (!cgi_expiry_queue.empty() && cgi_expiry_queue.top().getExpiration() <= now)
	{
		CGI*	cgi = cgi_expiry_queue.top().getCgi();
		cgi_expiry_queue.pop();
		if (!cgi->isFinished())
		{
			kill(cgi->getPid(), SIGKILL);
			waitpid(cgi->getPid(), NULL, 0);
			cgi->set_client_cgi_timed_out(true);
			cgi->make_client_response(*this);
			modifyEpoll(cgi->get_client_fd(), EPOLLOUT, EPOLL_CTL_MOD);
		}
		delete cgi;
	}
	if (!cgi_expiry_queue.empty())
	{
		cgi_timer.setTimer(cgi_expiry_queue.top().getExpiration() - now);
	}
}

void	Epoll::addCgiExpiry(CGI* cgi)
{
	time_t	now = time(NULL);
	cgi_expiry_queue.push(CGIExpiration(cgi, now + CGI_TIMEOUT));
	cgi_timer.setTimer(cgi_expiry_queue.top().getExpiration() - now);
}
