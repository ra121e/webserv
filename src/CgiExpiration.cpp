#include "../include/CGI.hpp"
#include "../include/CgiExpiration.hpp"

CGIExpiration::CGIExpiration(CGI* _cgi, time_t exp)
	: BaseExpiration(exp), cgi(_cgi)
{
}

CGI* CGIExpiration::getCgi() const
{
	return cgi;
}
