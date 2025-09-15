#ifndef DIRECTORY_HPP
#define DIRECTORY_HPP
#include <dirent.h>

class Directory
{
private:
	DIR*	dir;
	Directory(const Directory& other);
	Directory& operator=(const Directory& other);
public:
	Directory(const char* path);
	~Directory();

	struct dirent* read();
};

#endif
