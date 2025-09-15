#include "../include/FdExpiration.hpp"
#include <ctime>

FdExpiration::FdExpiration(int _fd, time_t exp)
	: BaseExpiration(exp), fd(_fd)
{
}

int FdExpiration::getFd() const
{
	return fd;
}
