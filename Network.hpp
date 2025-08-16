#ifndef NETWORK_HPP
#define NETWORK_HPP
#include <stdint.h>
#include <string>
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
	std::string	host;
	std::string	port;
	int			socket_fd;
	
public:
	Network();
	Network(const std::string& _host, const std::string& _port);
	Network(const Network& other);
	Network&	operator=(const Network& other);
	~Network();
	int		getFd() const;
	const std::string&	getHost() const;
	const std::string&	getPort() const;
	void	setupListener();
};

#endif
