#include "../include/CGI.hpp"
#include "../include/CgiExpiration.hpp"

CGIExpiration::CGIExpiration(const SharedPointer<CGI>& _cgi, time_t exp)
	: BaseExpiration(exp), cgi(_cgi)
{
}

const SharedPointer<CGI>& CGIExpiration::getCgi() const
{
	return cgi;
}

int	CGIExpiration::getServerReadFd() const
{
	return cgi->get_server_read_fd();
}
