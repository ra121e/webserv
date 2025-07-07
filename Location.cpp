#include "Location.hpp"

Location::Location()
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
