#ifndef NETWORK_HPP
#define NETWORK_HPP
#include <stdint.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>

class Network
{
private:
	const char	*host;
	const char	*port;
	int			socket_fd;
	
public:
	Network();
	Network(const char *_host, const char *_port);
	~Network();
	int		getFd() const;
	void	setupListener();
};

#endif
