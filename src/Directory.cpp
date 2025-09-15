#include "../include/Directory.hpp"
#include <cerrno>
#include <cstring>
#include <stdexcept>

Directory::Directory(const char* path): dir(opendir(path))
{
	if (!dir)
	{
		throw std::runtime_error("Failed to open directory: " + std::string(strerror(errno)));
	}
}

Directory::~Directory()
{
	closedir(dir);
}

struct dirent*	Directory::read()
{
	return readdir(dir);
}
