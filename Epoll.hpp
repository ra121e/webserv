#ifndef EPOLL_HPP
#define EPOLL_HPP
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <sys/epoll.h>

class Epoll
{
private:
	int	fd;

	Epoll(const Epoll& other);
	Epoll&	operator=(const Epoll& other);
public:
	Epoll(int _fd);
	~Epoll();
	int	getFd() const;
	void	addEventListener(int listen_fd) const;
};

#endif
