#include "main.hpp"
#include <string>

static void	parse_route_methods(std::istringstream& iss, Location& route)
{
	std::string	word;

	while (iss >> word)
	{
		if (word == "GET" || word == "POST" || word == "DELETE")
		route.addMethod(word);
	}
}

static bool	parse_route_attributes(const std::string& line, Location& route)
{
	std::istringstream	iss(line);
	std::string			word;

	if (iss >> word)
	{
		if (word == "methods")
			parse_route_methods(iss, route);
		else if (word == "alias")
		{
			if (!(iss >> word))
				throw std::ios_base::failure("Error: no alias provided");
			route.setAlias(word);
		}
		else if (word == "autoindex")
		{
			if (!(iss >> word))
				throw std::ios_base::failure("Error: autoindex not specified");
			if (word == "on")
				route.enableAutoIndex();
			else if (word != "off")
				throw std::runtime_error("Error: autoindex attribute must be either on or off");
		}
		else if (word == "index")
		{
			if (!(iss >> word))
				throw std::ios_base::failure("Error: no index provided");
//			route.setAlias(word);
			route.setIndex(word);
		}
		else if (word == "}")
			return false;
		else
			throw std::runtime_error("Error: unrecognized token '" + word + "'");
	}
	return true;
}

static void	parse_route(std::ifstream& infile, std::istringstream& ss, Server& serv)
{
	std::string	word;
	std::string	line;
	std::string	path;
	Location	route;

	if (ss >> word)
	{
		path = word;
		if (!(ss >> word) || word != "{")
			throw std::ios_base::failure("Error: expected '{'");
		while (std::getline(infile, line))
		{
			if (!parse_route_attributes(line, route))
				break;
		}
		serv.addLocation(path, route);
	}
	else
		throw std::ios_base::failure("Error: expected a route path");
}

static bool	parse_single_error_page(Server& serv, const std::string& line)
{
	std::istringstream	iss(line);
	std::string			error;
	std::string			page;

	if (!(iss >> error))
		return true;
	if (error == "}")
		return false;
	if (!(iss >> page))
		throw std::ios_base::failure("Error: expected [error_code] [error_page]");
	serv.addErrorPage(error, page);
	return true;
}

static void	parse_error_pages(std::ifstream& infile, std::istringstream& ss, Server& serv)
{
	std::string	word;
	std::string	line;

	if (ss >> word)
	{
		if (word != "{")
			throw std::runtime_error("Error: expected '{'");
		if (ss >> word)
			throw std::runtime_error("Error: unexpected token '" + word + "'");
		while (std::getline(infile, line))
		{
			if (!parse_single_error_page(serv, line))
				break;
		}
	}
	else
		throw std::ios_base::failure("Error: expected '{'");
}

static void	parse_client_max_body_size(std::istringstream &ss, Server& serv)
{
	std::string	word;

	if (ss >> word)
	{
		std::istringstream	iss(word);
		uint64_t			size = 0;
		if (!(iss >> size))
			throw std::ios_base::failure("Error: client_max_body_size must be a 64-bit integer");
		serv.setClientMaxBodySize(size);
	}
	else
		throw std::ios_base::failure("Error: Missing client_max_body_size number");
}

static void	parse_listen(std::istringstream &ss, Server& serv)
{
	std::string	word;

	if (ss >> word)
	{
		size_t	colonPos = word.find(':');
		if (colonPos == std::string::npos)
		{
			throw std::runtime_error("Error: interface:port expected");
		}
		std::string	hostStr = word.substr(0, colonPos);
		std::string	portStr = word.substr(colonPos + 1);
		serv.addNetwork(new Network(hostStr, portStr));
	}
	else
		throw std::ios_base::failure("Error: interface:port expected");
}

static void	parse_server(std::ifstream& infile, std::istringstream& iss, Config& conf)
{
	Server*		serv = new Server();
	std::string	word;

	iss >> word;
	if (!iss || word != "{")
		throw std::runtime_error("Error: expected '{'");
	iss >> word;
	if (iss)
		throw std::runtime_error("Error: unexpected token '" + word + "'");
	for (std::string line; std::getline(infile, line);)
	{
		std::istringstream	ss(line);

		if (ss >> word)
		{
			if (word == "listen")
				parse_listen(ss, *serv);
			else if (word == "client_max_body_size")
				parse_client_max_body_size(ss, *serv);
			else if (word == "error_page")
				parse_error_pages(infile, ss, *serv);
			else if (word == "location")
				parse_route(infile, ss, *serv);
			else if (word == "}")
				break;
			else
				throw std::runtime_error("Error: unexpected token '" + word + "'");
		}
	}
	conf.addServer(serv);
}

void	get_file_config(const char *filename, Config& conf)
{
	std::ifstream	infile(filename);

	if (!infile)
		throw std::ios_base::failure("Error: unable to open " + std::string(filename));
	for (std::string line; std::getline(infile, line);)
	{
		std::istringstream	iss(line);
		std::string			word;

		while (iss >> word)
		{
			if (word == "server")
				parse_server(infile, iss, conf);
			else
				throw std::runtime_error("Error: token '" + word + "' is not recognized");
		}
	}
}
