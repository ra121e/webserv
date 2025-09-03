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
#include "BaseFile.hpp"

class Network: public BaseFile
{
private:
	std::string	host;
	std::string	port;
	
public:
	Network();
	Network(const std::string& _host, const std::string& _port);
	const std::string&	getHost() const;
	const std::string&	getPort() const;
	void	setupListener();
};

#endif
