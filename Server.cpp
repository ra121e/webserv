#include "Server.hpp"

Server::Server()
{
}

Server::~Server()
{
}

void	Server::setHost(const std::string& _host)
{
	host = _host;
}

void	Server::setPort(uint16_t _port)
{
	port = _port;
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
