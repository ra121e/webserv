#include "Location.hpp"

Location::Location() : autoindex(false), is_redirect(false), redirect_code(302)
{
}

Location::~Location()
{
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

bool	Location::getIsRedirect() const
{
	return (is_redirect);
}

std::string Location::getRedirectTarget() const
{
	return (redirect_target);
}

int	Location::getRedirectCode() const
{
	return (redirect_code);
}
