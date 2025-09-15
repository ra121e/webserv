#ifndef FDEXPIRATION_HPP
#define FDEXPIRATION_HPP
#include <ctime>
#include "BaseExpiration.hpp"

class FdExpiration : public BaseExpiration
{
private:
	int 	fd;
public:
	FdExpiration(int _fd, time_t exp);

	int		getFd() const;
};

#endif
