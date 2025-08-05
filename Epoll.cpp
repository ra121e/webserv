#include "Epoll.hpp"

Epoll::Epoll(int _fd): fd(_fd)
{
	if (fd == -1)
	{
		throw std::runtime_error(strerror(errno));
	}
}

Epoll::~Epoll()
{
	close(fd);
}

int	Epoll::getFd() const
{
	return fd;
}

void	Epoll::addEventListener(int listen_fd) const
{
	// set server fd and event into event struct (its like "entry sheet" or "application form")
	struct epoll_event	event = {};
	event.events = EPOLLIN | EPOLLET;
	event.data.fd = listen_fd;

	// registration event into epoll
	if (epoll_ctl(fd, EPOLL_CTL_ADD, listen_fd, &event) == -1)
	{
		throw std::runtime_error(strerror(errno));
	}
}
