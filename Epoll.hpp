#ifndef EPOLL_HPP
#define EPOLL_HPP
#include <map>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <sys/epoll.h>
#include <vector>
#include <netinet/in.h>
#include "ClientConnection.hpp"

class Epoll
{
private:
	int	fd;
	std::vector<int>	server_fds;
	std::map<int, ClientConnection*>	clients;
	static const int	MAX_EVENTS = 64;
	struct epoll_event	events[MAX_EVENTS];

	Epoll(const Epoll& other);
	Epoll&	operator=(const Epoll& other);
public:
	Epoll(int _fd);
	~Epoll();
	int	getFd() const;
	void	addEventListener(int listen_fd);
	void	addClient(int server_fd);
	void	modifyEventListener(ClientConnection *client);
	void	handleEvents();
};

#endif
