#include "Config.hpp"
#include <vector>

Config::Config()
{
}

Config::~Config()
{
	for (std::vector<Server*>::iterator it = servers.begin(); it != servers.end(); ++it)
	{
		delete *it;
	}
}

void	Config::addServer(Server* server)
{
	servers.push_back(server);
}

void	Config::setupServers()
{
	for (std::vector<Server*>::iterator it = servers.begin(); it != servers.end(); ++it)
	{
		(*it)->setup();
	}
}

const std::vector<Server*>&	Config::getServers() const
{
	return servers;
}
