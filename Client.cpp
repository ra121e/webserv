#include "Client.hpp"
#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <unistd.h>
#include <fcntl.h>

Client::Client()
{
}

Client::Client(int _fd): fd(_fd)
{
	if (fd < 0)
	{
		throw std::invalid_argument(strerror(errno));
	}
}

Client::Client(const Client& other): fd(dup(other.fd))
{
}

Client&	Client::operator=(const Client& other)
{
	if (this != &other)
	{
		fd = dup(other.fd);
	}
	return *this;
}

Client::~Client()
{
	close(fd);
}

int	Client::getFd() const
{
	return fd;
}

void	Client::setNonBlocking() const
{
	// setting the new client socket as previous
	// change client socket to non-blocking mode
	if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1)
	{
		throw std::runtime_error(strerror(errno));
	}
}
