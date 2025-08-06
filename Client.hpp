#ifndef CLIENT_HPP
#define CLIENT_HPP

class Client
{
private:
	int	fd;
public:
	Client();
	Client(int _fd);
	Client(const Client& other);
	Client&	operator=(const Client& other);
	~Client();
	int	getFd() const;
	void	setNonBlocking() const;
};

#endif
