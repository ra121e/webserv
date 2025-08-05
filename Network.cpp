#include "Network.hpp"
#include <asm-generic/socket.h>
#include <cstring>
#include <netdb.h>
#include <stdexcept>
#include <sys/socket.h>

Network::Network()
{
}

Network::Network(const char *_host, const char *_port): host(_host), port(_port), socket_fd(-1)
{
}

Network::~Network()
{
	close(socket_fd);
}

int	Network::getFd() const
{
	return socket_fd;
}

void	Network::setupListener()
{
	struct addrinfo	hints = {};
	struct addrinfo	*res = NULL;
	const int	BACKLOG = 10;
	int	yes = 1;

	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM | SOCK_CLOEXEC;
	hints.ai_flags = AI_PASSIVE;

	int	gaiValue = getaddrinfo(host, port, &hints, &res);
	if (gaiValue != 0)
	{
		throw std::runtime_error(gai_strerror(gaiValue));
	}
	for (struct addrinfo* ai = res; ai; ai = ai->ai_next)
	{
		socket_fd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
		setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
		if (socket_fd < 0)
		{
			throw std::runtime_error(strerror(errno));
		}
		if (bind(socket_fd, ai->ai_addr, ai->ai_addrlen) < 0)
		{
			throw std::runtime_error(strerror(errno));
		}
		if (listen(socket_fd, BACKLOG) < 0)
		{
			throw std::runtime_error(strerror(errno));
		}
		freeaddrinfo(res);
	}
}
