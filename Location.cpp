#include "Location.hpp"
#include <sstream>
#include <string>

Location::Location() : autoindex(false), is_redirect(false), redirect_code(0)
{
}

void	Location::parse_route_alias(std::istringstream& iss)
{
	std::string ali;
	if (!(iss >> ali))
	{
		throw std::ios_base::failure("Error: no alias provided");
	}
	setAlias(ali);
}

void	Location::parse_route_autoindex(std::istringstream& iss)
{
	std::string word;
	if (!(iss >> word))
	{
		throw std::ios_base::failure("Error: autoindex not specified");
	}
	if (word == "on")
	{
		enableAutoIndex();
	}
	else if (word != "off")
	{
		throw std::runtime_error("Error: autoindex attribute must be either on or off");
	}
}

void	Location::parse_route_index(std::istringstream& iss)
{
	std::string idx;
	if (!(iss >> idx))
	{
		throw std::ios_base::failure("Error: no index provided");
	}
	setIndex(idx);
}

void	Location::parse_route_redirect(std::istringstream& iss)
{
	int			code = 0;
	std::string target;

	if (!(iss >> code) || !(iss >> target))
	{
		throw std::ios_base::failure("Error: redirect directive must have [status_code] [target_url]");
	}
	setRedirect(target, code);
}

bool	Location::parse_route_attributes(const std::string& line)
{
	std::istringstream	iss(line);
	std::string			word;

	if ((iss >> word) != 0)
	{
		if (word == "methods")
		{
			parse_route_methods(iss);
		}
		else if (word == "alias")
		{
			parse_route_alias(iss);
		}
		else if (word == "autoindex")
		{
			parse_route_autoindex(iss);
		}
		else if (word == "index")
		{
			parse_route_index(iss);
		}
		else if (word == "redirect")
		{
			parse_route_redirect(iss);
		}
		else if (word == "}")
		{
			return false;
		}
		else
		{
			throw std::runtime_error("Error: unrecognized token '" + word + "'");
		}
	}
	return true;
}

void	Location::parse_route_methods(std::istringstream& iss)
{
	std::string	word;

	while ((iss >> word) != 0)
	{
		if (word == "GET" || word == "POST" || word == "DELETE")
		{
			addMethod(word);
		}
	}
}

void	Location::addMethod(const std::string& method)
{
	methods.push_back(method);
}

void	Location::setAlias(const std::string& _alias)
{
	alias = _alias;
}

void	Location::enableAutoIndex()
{
	autoindex = true;
}

void	Location::setIndex(const std::string& _index)
{
	index = _index;
}

void	Location::setRedirect(const std::string &target, int code)
{
	is_redirect = true;
	redirect_target = target;
	redirect_code = code;
}

bool	Location::isMethod(std::string const &method) const
{
	for (size_t i = 0; i < methods.size(); ++i)
	{
		if (methods[i] == method)
			return (true);
	}
	return (false);
}

bool	Location::isAutoindexOn() const
{
	return (autoindex);
}

const std::string	&Location::getAlias() const
{
	return (alias);
}

const std::string	&Location::getIndex() const
{
	return (index);
}

const std::vector<std::string>& Location::getMethods() const
{
	return methods;
}

bool	Location::getIsRedirect() const
{
	return (is_redirect);
}

const std::string& Location::getRedirectTarget() const
{
	return (redirect_target);
}

int	Location::getRedirectCode() const
{
	return (redirect_code);
}
