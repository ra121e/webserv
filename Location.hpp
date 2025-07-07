#ifndef LOCATION_HPP
#define LOCATION_HPP
#include <vector>
#include <iostream>

class Location
{
private:
	std::vector<std::string>	methods;
	std::string					alias;
	bool						autoindex;
	std::string					index;
public:
	Location();
	~Location();
	void	addMethod(const std::string& method);
	void	setAlias(const std::string& _alias);
	void	enableAutoIndex();
	void	setIndex(const std::string& index);
};

#endif
