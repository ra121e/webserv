#ifndef SERVER_HPP
#define SERVER_HPP

#include <sys/types.h>
#include <utility>
#include <vector>
#include <map>
#include <iostream>

#include <fstream>
#include <sstream>
#include "Location.hpp"
#include "Network.hpp"

class Server
{
private:
	std::vector<Network*>				networks;
	uint64_t							client_max_body_size;
	std::map<std::string, std::string>	error_pages;
	std::map<std::string, Location>		locations;
	
public:
	Server();
	Server(const Server& other);
	Server&	operator=(const Server& other);
	~Server();
	void	addNetwork(Network* net);
	void	setClientMaxBodySize(uint64_t _client_max_body_size);
	void	addErrorPage(const std::string& error, const std::string& page);
	void	addLocation(const std::string& path, const Location& location);
	const Location&	getLocation(std::string const &uri) const;
	const std::map<std::string, Location> &getLocations() const;
	void	setup();
	const std::vector<Network*>&	getNetworks() const;
	std::string getErrorPage(int code) const;
	uint64_t getClientMaxBodySize() const;
};

#endif
