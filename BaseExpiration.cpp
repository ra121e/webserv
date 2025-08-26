#include "BaseExpiration.hpp"

BaseExpiration::BaseExpiration() : expiration(0) {}

BaseExpiration::BaseExpiration(time_t exp) : expiration(exp) {}

BaseExpiration::BaseExpiration(const BaseExpiration& other) : expiration(other.expiration) {}

BaseExpiration&	BaseExpiration::operator=(const BaseExpiration& other)
{
	if (this != &other)
	{
		expiration = other.expiration;
	}
	return *this;
}

BaseExpiration::~BaseExpiration() {}

void	BaseExpiration::setExpiration(time_t exp)
{
	expiration = exp;
}

time_t	BaseExpiration::getExpiration() const
{
	return expiration;
}
