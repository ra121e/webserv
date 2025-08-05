#ifndef CONFIG_HPP
#define CONFIG_HPP
#include <vector>
#include "Server.hpp"

class Config
{
private:
	std::vector<Server>	servers;
public:
	Config();
	~Config();
	void	addServer(const Server& server);
	void	setupServers();
	const std::vector<Server>&	getServers() const;
};

#endif
