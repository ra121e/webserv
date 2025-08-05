#include "Server.hpp"
#include <netdb.h>
#include <stdexcept>
#include <sys/socket.h>
#include <vector>

Server::Server()
{
}

Server::~Server()
{
}

void	Server::addNetwork(const Network& net)
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

void	Server::setup()
{
	for (std::vector<Network>::iterator it = networks.begin(); it != networks.end(); ++it)
	{
		it->setupListener();
	}
	// preparation of server socket
	int	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd < 0)
	{
		throw std::runtime_error("Failed to create socket");
	}
	// std::cout << "socket return value is " << server_fd << std::endl;

	// change server socket to non-blocking mode
	// if (fcntl(server_fd, F_SETFL, O_NONBLOCK) == -1)
	// {
	// 	perror("fcntl NONBLOCK");
	// 	close(server_fd);
	// 	return (1);
	// }
}

const std::vector<Network>&	Server::getNetworks() const
{
	return networks;
}
