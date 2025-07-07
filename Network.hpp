#ifndef NETWORK_HPP
#define NETWORK_HPP
#include <iostream>

class Network
{
private:
	std::string	host;
	uint16_t	port;
public:
	Network();
	Network(const std::string& _host, uint16_t _port);
	~Network();
};

#endif
