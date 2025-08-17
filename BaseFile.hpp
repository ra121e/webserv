#ifndef BASEFILE_HPP
#define BASEFILE_HPP

class BaseFile
{
private:
	int fd; // file descriptor

public:
	BaseFile(int _fd);
	BaseFile(const BaseFile& other);
	BaseFile& operator=(const BaseFile& other);
	virtual ~BaseFile() = 0;

	int getFd() const;
};

#endif
