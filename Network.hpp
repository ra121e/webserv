#ifndef NETWORK_HPP
#define NETWORK_HPP
#include <iostream>
#include <stdint.h>

struct Network
{
	std::string	host;
	uint16_t	port;
	
	Network();
	Network(const std::string& _host, uint16_t _port);
	~Network();
};

#endif
