#include "Config.hpp"

Config::Config()
{
}

Config::~Config()
{
}

void	Config::addServer(const Server& server)
{
	servers.push_back(server);
}
