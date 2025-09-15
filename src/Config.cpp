#include "../include/Config.hpp"
#include <vector>

Config::Config()
{
}

void	Config::addServer(const SharedPointer<Server>& server)
{
	servers.push_back(server);
}

void	Config::setupServers()
{
	for (std::vector<SharedPointer<Server> >::iterator it = servers.begin(); it != servers.end(); ++it)
	{
		(*it)->setup();
	}
}

const std::vector<SharedPointer<Server> >&	Config::getServers() const
{
	return servers;
}

void	Config::get_file_config(const char *filename)
{
	std::ifstream	infile(filename);

	if (!infile)
	{
		throw std::ios_base::failure("Error: unable to open " + std::string(filename));
	}
	for (std::string line; std::getline(infile, line) != 0;)
	{
		std::istringstream	iss(line);
		std::string			word;

		while ((iss >> word) != 0)
		{
			if (word == "server")
			{
				parse_server(infile, iss);
			}
			else
			{
				throw std::runtime_error("Error: token '" + word + "' is not recognized");
			}
		}
	}
}

void	Config::parse_server(std::ifstream& infile, std::istringstream& iss)
{
	SharedPointer<Server> serv(new Server);
	std::string	word;

	iss >> word;
	if (!iss || word != "{")
	{
		throw std::runtime_error("Error: expected '{'");
	}
	iss >> word;
	if (iss != 0)
	{
		throw std::runtime_error("Error: unexpected token '" + word + "'");
	}
	for (std::string line; std::getline(infile, line) != 0;)
	{
		std::istringstream	ss(line);

		if ((ss >> word) != 0)
		{
			if (word == "listen")
			{
				serv->parse_listen(ss);
			}
			else if (word == "client_max_body_size")
			{
				serv->parse_client_max_body_size(ss);
			}
			else if (word == "error_page")
			{
				serv->parse_error_pages(infile, ss);
			}
			else if (word == "location")
			{
				serv->parse_route(infile, ss);
			}
			else if (word == "}")
			{
				break;
			}
			else
			{
				throw std::runtime_error("Error: unexpected token '" + word + "'");
			}
		}
	}
	addServer(serv);
}
