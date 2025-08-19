#ifndef EPOLL_HPP
#define EPOLL_HPP
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

class Epoll : public BaseFile
{
private:
	std::map<int, Server*>	servers;
	std::map<int, ClientConnection*>	clients;
	static const int	MAX_EVENTS = 64;
	struct epoll_event	events[MAX_EVENTS];
	Timer	timer;
	static const time_t	TIMEOUT = 10;
	std::priority_queue<ConnectionExpiration, std::vector<ConnectionExpiration>, std::greater<ConnectionExpiration> >	expiry_queue;

	Epoll(const Epoll& other);
	Epoll&	operator=(const Epoll& other);
public:
	Epoll(int _fd);
	~Epoll();
	void	addEventListener(Server* server, int listen_fd);
	void	modifyEventListener(ClientConnection *client) const;
	void	addClient(int server_fd);
	void	handleEvents();
	void	addTimer();
};

#endif
