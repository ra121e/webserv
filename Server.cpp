#include "Server.hpp"
#include <cerrno>
#include <cstddef>
#include <cstring>
#include <netdb.h>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <utility>
#include <vector>
#include <fcntl.h>

Server::Server()
{
	Location cgiLoc;
	cgiLoc.setAlias("cgi-bin/"); // actual disk path to CGI scripts
	cgiLoc.addMethod("GET");
	cgiLoc.addMethod("POST");
	cgiLoc.addMethod("DELETE");
	locations["/cgi-bin/"] = cgiLoc;
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

const Location&	Server::getLocation(std::string const &uri) const
{
	std::string clean_uri = uri;
	size_t pos = clean_uri.find('?');
	if (pos != std::string::npos)
		clean_uri.erase(pos);
		
	std::cout << "Original Uri: " << uri << std::endl;
	std::cout << "Clean URI: " << clean_uri << std::endl;
	
	for (std::map<std::string, Location>::const_iterator it = locations.begin(); it != locations.end(); ++it)
	{
		std::cout << "it->first " << it->first << std::endl;
		const std::string& path = it->first;
		if (path[path.size() - 1] == '/' && path != "/")
		{
			size_t	final_slash_pos = clean_uri.rfind("/");
			if (path == clean_uri.substr(0, final_slash_pos + 1))
			{
				return it->second;
			}
		}
		else if (path == clean_uri)
		{
			std::cout << "Location exact match: path=[" << path 
              << "], uri=[" << clean_uri << "]" << std::endl;
			return it->second;
		}
	}
	std::cerr << "No Location match for URI: [" << uri << "]" << std::endl;
	throw std::runtime_error("Location not found");
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

std::string Server::getErrorPage(int code) const
{
	std::ostringstream oss;
	oss << code;
	std::string codeStr = oss.str();
	std::map<std::string, std::string>::const_iterator it = error_pages.find(codeStr);
	if (it != error_pages.end())
	{
		return it->second;
	}
	return "";
}
