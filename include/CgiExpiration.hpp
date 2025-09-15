#ifndef CGI_EXPIRATION_HPP
#define CGI_EXPIRATION_HPP
#include <ctime>
#include "CGI.hpp"
#include "BaseExpiration.hpp"
#include "SharedPointer.hpp"

class CGIExpiration : public BaseExpiration
{
private:
	SharedPointer<CGI> cgi;
public:
	CGIExpiration(const SharedPointer<CGI>& cgi, time_t exp);
	const SharedPointer<CGI>& getCgi() const;
	int		getServerReadFd() const;
};

#endif
