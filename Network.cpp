#include "Network.hpp"
#include <asm-generic/socket.h>
#include <cerrno>
#include <cstring>
#include <netdb.h>
#include <stdexcept>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>

Network::Network()
{
}

Network::Network(const std::string& _host, const std::string& _port): host(_host), port(_port), socket_fd(-1)
{
}

Network::Network(const Network& other): host(other.host), port(other.port), socket_fd(dup(other.socket_fd))
{
}

Network&	Network::operator=(const Network& other)
{
	if (this != &other)
	{
		host = other.host;
		port = other.port;
		socket_fd = dup(other.socket_fd);
	}
	return *this;
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
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_V4MAPPED | AI_ADDRCONFIG;

	int	gaiValue = getaddrinfo(host.c_str(), port.c_str(), &hints, &res);
	if (gaiValue != 0)
	{
		throw std::runtime_error(gai_strerror(gaiValue));
	}
	for (struct addrinfo* ai = res; ai; ai = ai->ai_next)
	{
		socket_fd = socket(ai->ai_family, ai->ai_socktype | SOCK_CLOEXEC, ai->ai_protocol);
		if (socket_fd < 0)
		{
			continue;
		}
		setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
		if (fcntl(socket_fd, F_SETFL, O_NONBLOCK) < 0)
		{
			close(socket_fd);
			continue;
		}
		if (bind(socket_fd, ai->ai_addr, ai->ai_addrlen) < 0)
		{
			close(socket_fd);
			continue;
		}
		if (listen(socket_fd, BACKLOG) < 0)
		{
			close(socket_fd);
			continue;
		}
		freeaddrinfo(res);
		return;
	}
	freeaddrinfo(res);
	throw std::runtime_error("Unable to bind socket to given host or port");
}
