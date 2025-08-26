#ifndef BASEEXPIRATION_HPP
#define BASEEXPIRATION_HPP
#include <ctime>

class BaseExpiration
{
private:
	time_t	expiration;
public:
	BaseExpiration();
	BaseExpiration(time_t exp);
	BaseExpiration(const BaseExpiration& other);
	BaseExpiration&	operator=(const BaseExpiration& other);
	virtual ~BaseExpiration() = 0;
	void	setExpiration(time_t exp);
	time_t	getExpiration() const;
};

#endif
