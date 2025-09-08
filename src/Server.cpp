#include "../include/Server.hpp"
#include "../include/Network.hpp"
#include <cerrno>
#include <cstddef>
#include <cstring>
#include <map>
#include <netdb.h>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <utility>
#include <vector>
#include <fcntl.h>
#include <algorithm>

Server::Server() : client_max_body_size()
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
		for (std::vector<Network*>::const_iterator it = networks.begin(); it != networks.end(); ++it)
		{
			delete *it;
		}
		networks.clear();
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

void	Server::parse_listen(std::istringstream &ss)
{
	std::string	word;

	if (ss >> word)
	{
		size_t	colonPos = word.find(':');
		if (colonPos == std::string::npos)
		{
			throw std::runtime_error("Error: interface:port expected");
		}
		const std::string&	hostStr = word.substr(0, colonPos);
		const std::string&	portStr = word.substr(colonPos + 1);
		addNetwork(new Network(hostStr, portStr));
	}
	else
	{
		throw std::ios_base::failure("Error: interface:port expected");
	}
}

void	Server::parse_client_max_body_size(std::istringstream &ss)
{
	std::string	word;

	if (ss >> word)
	{
		std::istringstream	iss(word);
		uint64_t			size = 0;
		if (!(iss >> size))
		{
			throw std::ios_base::failure("Error: client_max_body_size must be a 64-bit integer");
		}
		setClientMaxBodySize(size);
	}
	else
	{
		throw std::ios_base::failure("Error: Missing client_max_body_size number");
	}
}

void	Server::parse_error_pages(std::ifstream& infile, std::istringstream& ss)
{
	std::string	word;
	std::string	line;

	if (ss >> word)
	{
		if (word != "{")
		{
			throw std::runtime_error("Error: expected '{'");
		}
		if (ss >> word)
		{
			throw std::runtime_error("Error: unexpected token '" + word + "'");
		}
		while (std::getline(infile, line) != 0)
		{
			if (!parse_single_error_page(line))
			{
				break;
			}
		}
	}
	else
	{
		throw std::ios_base::failure("Error: expected '{'");
	}
}

bool	Server::parse_single_error_page(const std::string& line)
{
	std::istringstream	iss(line);
	std::string			error;
	std::string			page;

	if (!(iss >> error))
	{
		return true;
	}
	if (error == "}")
	{
		return false;
	}
	if (!(iss >> page))
	{
		throw std::ios_base::failure("Error: expected [error_code] [error_page]");
	}
	addErrorPage(error, page);
	return true;
}

void	Server::parse_route(std::ifstream& infile, std::istringstream& ss)
{
	std::string	word;
	std::string	line;
	std::string	path;
	Location	route;

	if (ss >> word)
	{
		path = word;
		if (!(ss >> word) || word != "{")
		{
			throw std::ios_base::failure("Error: expected '{'");
		}
		while (std::getline(infile, line) != 0)
		{
			if (!route.parse_route_attributes(line))
				break;
		}
		addLocation(path, route);
	}
	else
		throw std::ios_base::failure("Error: expected a route path");
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

std::map<std::string, Location>::const_iterator	Server::getLocationIteratorMatch(std::string const &uri, const std::string& extension, HttpRequest& request) const
{
	std::string clean_uri = uri;
	size_t pos = clean_uri.find('?');
	if (pos != std::string::npos)
		clean_uri.erase(pos);
	for (std::map<std::string, Location>::const_iterator it = locations.begin(); it != locations.end(); ++it)
	{
		const std::string& path = it->first;
		if (!extension.empty() && it->second.supports_cgi_extension(extension))
		{
			request.forward_to_cgi = true;
			return it;
		}
		if (path[path.size() - 1] == '/' && path != "/")
		{
			size_t	final_slash_pos = clean_uri.rfind("/");
			if (path == clean_uri.substr(0, final_slash_pos + 1))
			{
				return it;
			}
		}
		else if (path == clean_uri)
		{
			return it;
		}
	}
	return locations.end();
}

const std::map<std::string, Location> &Server::getLocations() const
{
	return locations;
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

uint64_t Server::getClientMaxBodySize() const
{
	return client_max_body_size;
}

bool	Server::addUser(const std::string& username, const std::string& password)
{
	for (std::vector<User>::const_iterator it = users.begin(); it != users.end(); ++it)
	{
		if (it->getUsername() == username)
		{
			return false;
		}
	}
	users.push_back(User(username, password));
	return true;
}

void	Server::addSessionId(const std::string& session_id)
{
	session_ids.push_back(session_id);
}

bool	Server::isValidSessionId(const std::string& session_id) const
{
	return std::find(session_ids.begin(), session_ids.end(), session_id) != session_ids.end();
}

bool	Server::authenticateUser(const std::string& username, const std::string& password) const
{
	User userToFind(username, password);
	return std::find(users.begin(), users.end(), userToFind) != users.end();
}

void	Server::removeSessionId(const std::string& session_id)
{
	std::vector<std::string>::iterator it = std::find(session_ids.begin(),
	session_ids.end(),
	session_id);
	if (it != session_ids.end())
	{
		session_ids.erase(it);
	}
}
