#ifndef CONFIG_HPP
#define CONFIG_HPP
#include <vector>
#include "Server.hpp"
#include "SharedPointer.hpp"

class Config
{
private:
	std::vector<SharedPointer<Server> >	servers;
	Config(const Config& other);
	Config&	operator=(const Config& other);
public:
	Config();
	void	addServer(const SharedPointer<Server>& server);
	void	setupServers();
	const std::vector<SharedPointer<Server> >&	getServers() const;
	void	get_file_config(const char *filename);
	void	parse_server(std::ifstream& infile, std::istringstream& iss);
};

#endif
