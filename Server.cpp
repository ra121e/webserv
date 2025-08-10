#include "Server.hpp"
#include <cerrno>
#include <cstring>
#include <netdb.h>
#include <stdexcept>
#include <sys/socket.h>
#include <vector>
#include <fcntl.h>

Server::Server()
{
}

Server::Server(const Server& other): client_max_body_size(other.client_max_body_size), error_pages(other.error_pages), locations(other.locations)
{
	for (std::vector<Network*>::const_iterator it = other.networks.begin(); it != other.networks.end(); ++it)
	{
		networks.push_back(new Network(**it));
	}
}

Server&	Server::operator=(const Server& other)
{
	if (this != &other)
	{
		for (std::vector<Network*>::const_iterator it = other.networks.begin(); it != other.networks.end(); ++it)
		{
			networks.push_back(new Network(**it));
		}
		client_max_body_size = other.client_max_body_size;
		error_pages = other.error_pages;
		locations = other.locations;
	}
	return *this;
}

Server::~Server()
{
	for (std::vector<Network*>::iterator it = networks.begin(); it != networks.end(); ++it)
	{
		delete *it;
	}
}

void	Server::addNetwork(Network* net)
{
	networks.push_back(net);
}

void	Server::setClientMaxBodySize(uint64_t _clientMaxBodySize)
{
	client_max_body_size = _clientMaxBodySize;
}

void	Server::addErrorPage(const std::string& error, const std::string& page)
{
	error_pages[error] = page;
}

void	Server::addLocation(const std::string& path, const Location& location)
{
	locations[path] = location;
}

std::pair<Location*, size_t>	Server::getLocation(std::string const &uri)
{
	Location	*match = NULL;
	size_t		longest = 0;

	std::cout << "uri: " << uri << std::endl;

	for (std::map<std::string, Location>::iterator it = locations.begin(); it != locations.end(); ++it)
	{
		const std::string	&path = it->first;

		std::cout << "Location: " << it->first << " " << it->second.getAlias() << std::endl;
		if (uri.find(path) == 0 && path.length() > longest)
		{
			match = &it->second;
			longest = path.length();
			std::cout << "Match is: " << it->first << match->getIndex() << std::endl;
		}
	}
	return (std::make_pair(match, longest));
}

void	Server::setup()
{
	for (std::vector<Network*>::iterator it = networks.begin(); it != networks.end(); ++it)
	{
		(*it)->setupListener();
	}
}

const std::vector<Network*>&	Server::getNetworks() const
{
	return networks;
}
