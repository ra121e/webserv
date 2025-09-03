#include "../include/Network.hpp"
#include <asm-generic/socket.h>
#include <cerrno>
#include <cstring>
#include <netdb.h>
#include <stdexcept>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>

Network::Network(const std::string& _host, const std::string& _port): host(_host), port(_port)
{
}

const std::string&	Network::getHost() const
{
	return host;
}

const std::string&	Network::getPort() const
{
	return port;
}

void	Network::setupListener()
{
	struct addrinfo	hints = {};
	struct addrinfo	*res = NULL;
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
		setFd(socket(ai->ai_family, ai->ai_socktype | SOCK_CLOEXEC | SOCK_NONBLOCK, ai->ai_protocol));
		if (getFd() < 0)
		{
			continue;
		}
		setsockopt(getFd(), SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
		if (bind(getFd(), ai->ai_addr, ai->ai_addrlen) < 0)
		{
			closeFd();
			continue;
		}
		if (listen(getFd(), SOMAXCONN) < 0)
		{
			closeFd();
			continue;
		}
		freeaddrinfo(res);
		return;
	}
	freeaddrinfo(res);
	throw std::runtime_error("Unable to bind socket to given host or port");
}
