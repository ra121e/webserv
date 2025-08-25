#include "Pipe.hpp"
#include "BaseFile.hpp"
#include <stdexcept>
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>

Pipe::Pipe()
{
	int fds[2];
	if (pipe2(fds, O_NONBLOCK | O_CLOEXEC) == -1)
	{
		throw std::runtime_error("pipe2: " + std::string(strerror(errno)));
	}
	read_end.setFd(fds[0]);
	write_end.setFd(fds[1]);
}

const BaseFile&	Pipe::operator[](int index) const
{
	if (index == 0)
	{
		return read_end;
	}
	if (index == 1)
	{
		return write_end;
	}
	throw std::out_of_range("Index out of range for Pipe");
}

BaseFile&	Pipe::operator[](int index)
{
	if (index == 0)
	{
		return read_end;
	}
	if (index == 1)
	{
		return write_end;
	}
	throw std::out_of_range("Index out of range for Pipe");
}
