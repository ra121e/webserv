#ifndef CGI_EXPIRATION_HPP
#define CGI_EXPIRATION_HPP
#include <ctime>
#include "CGI.hpp"
#include "BaseExpiration.hpp"

class CGIExpiration : public BaseExpiration
{
private:
	CGI* 	cgi;
public:
	CGIExpiration(CGI* cgi, time_t exp);
	CGI*	getCgi() const;
};

#endif
