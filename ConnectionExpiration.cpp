#include "ConnectionExpiration.hpp"
#include <ctime>

ConnectionExpiration::ConnectionExpiration(int fd, time_t exp)
	: client_fd(fd), expiration(exp)
{
}

bool ConnectionExpiration::operator>(const ConnectionExpiration& other) const
{
	return expiration > other.expiration;
}

int ConnectionExpiration::getFd() const
{
	return client_fd;
}

time_t ConnectionExpiration::getExpiration() const
{
	return expiration;
}
