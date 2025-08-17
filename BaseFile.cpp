#include "BaseFile.hpp"
#include <unistd.h>
#include <stdexcept>
#include <cstring>
#include <cerrno>

BaseFile::BaseFile(int _fd) : fd(_fd)
{
	if (fd < 0)
	{
		throw std::invalid_argument(strerror(errno));
	}
}

BaseFile::BaseFile(const BaseFile& other) : fd(dup(other.fd))
{
	if (fd < 0)
	{
		throw std::invalid_argument(strerror(errno));
	}
}

BaseFile& BaseFile::operator=(const BaseFile& other)
{
	if (this != &other)
	{
		if (fd >= 0)
		{
			close(fd);
		}
		fd = dup(other.fd);
		if (fd < 0)
		{
			throw std::invalid_argument(strerror(errno));
		}
	}
	return *this;
}

BaseFile::~BaseFile()
{
	close(fd);
}

int BaseFile::getFd() const
{
	return fd;
}
