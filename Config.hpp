#ifndef CONFIG_HPP
#define CONFIG_HPP
#include <vector>
#include "Server.hpp"

class Config
{
private:
	std::vector<Server*>	servers;
	Config(const Config& other);
	Config&	operator=(const Config& other);
public:
	Config();
	~Config();
	void	addServer(Server* server);
	void	setupServers();
	const std::vector<Server*>&	getServers() const;
};

#endif
