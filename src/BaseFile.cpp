#include "../include/BaseFile.hpp"
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

BaseFile::BaseFile() : fd(-1)
{
}

BaseFile::BaseFile(const BaseFile& other) : fd(dup(other.fd))
{
}

BaseFile& BaseFile::operator=(const BaseFile& other)
{
	if (this != &other)
	{
		closeFd();
		fd = dup(other.fd);
	}
	return *this;
}

BaseFile::~BaseFile()
{
	closeFd();
}

int BaseFile::getFd() const
{
	return fd;
}

int& BaseFile::getFd()
{
	return fd;
}

void BaseFile::setFd(int _fd)
{
	fd = _fd;
	if (fd < 0)
	{
		throw std::invalid_argument("Setting invalid file descriptor in BaseFile");
	}
}

void BaseFile::closeFd()
{
	if (fd >= 0)
	{
		close(fd);
		fd = -1;
	}
}
