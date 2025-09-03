#ifndef CONNECTIONEXPIRATION_HPP
#define CONNECTIONEXPIRATION_HPP
#include <ctime>

class ConnectionExpiration
{
private:
	int 	client_fd;
	time_t	expiration;
public:
	ConnectionExpiration(int fd, time_t exp);

	bool	operator>(const ConnectionExpiration& other) const;
	int		getFd() const;
	time_t	getExpiration() const;
};

#endif
