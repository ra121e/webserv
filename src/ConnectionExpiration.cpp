#include "../include/ConnectionExpiration.hpp"
#include <ctime>

ConnectionExpiration::ConnectionExpiration(int fd, time_t exp)
	: BaseExpiration(exp), client_fd(fd)
{
}

int ConnectionExpiration::getFd() const
{
	return client_fd;
}
