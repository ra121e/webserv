#ifndef CONNECTIONEXPIRATION_HPP
#define CONNECTIONEXPIRATION_HPP
#include <ctime>
#include "BaseExpiration.hpp"

class ConnectionExpiration : public BaseExpiration
{
private:
	int 	client_fd;
public:
	ConnectionExpiration(int fd, time_t exp);

	int		getFd() const;
};

#endif
