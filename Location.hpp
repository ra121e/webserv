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
};

#endif
