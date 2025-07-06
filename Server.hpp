#ifndef SERVER_HPP
#define SERVER_HPP
#include <poll.h>
#include <vector>
#include <map>
#include <iostream>
#include <sys/socket.h> // socket
#include <sys/types.h> // accept()
#include <netinet/in.h> // AF_INET, sockaddr_in type struct, INADDR_ANY
#include <errno.h> // perror
#include <stdio.h> // perror
#include <unistd.h> // close(sock)
#include <stdlib.h> // memset
#include <arpa/inet.h>
#include <string.h> // memset
#include <fstream>
#include <sstream>
#include "Location.hpp"

class Server
{
private:
	std::string							host;
	uint16_t							port;
	uint64_t							client_max_body_size;
	std::map<std::string, std::string>	error_pages;
	std::map<std::string, Location>		locations;
public:
	Server();
	~Server();
	void	setHost(const std::string& _host);
	void	setPort(uint16_t _port);
	void	setClientMaxBodySize(uint64_t _client_max_body_size);
	void	addErrorPage(const std::string& error, const std::string& page);
	void	addLocation(const std::string& path, const Location& location);
};

#endif
