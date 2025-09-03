#ifndef BASEFILE_HPP
#define BASEFILE_HPP

class BaseFile
{
private:
	int fd; // file descriptor

public:
	BaseFile();
	BaseFile(int _fd);
	BaseFile(const BaseFile& other);
	BaseFile& operator=(const BaseFile& other);
	virtual ~BaseFile();

	int		getFd() const;
	int&	getFd();
	void	setFd(int _fd);
	void	closeFd();
};

#endif
