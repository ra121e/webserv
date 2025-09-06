#include "../include/Exceptions.hpp"

const char*	ResourceNotFoundException::what() const throw()
{
	return "Resource Not Found Exception";
}

const char*	MethodNotAllowedException::what() const throw()
{
	return "Method Not Allowed Exception";
}

const char*	InternalServerErrorException::what() const throw()
{
	return "Internal Server Error Exception";
}

const char*	BadRequestException::what() const throw()
{
	return "Bad Request Exception";
}

const char*	PayloadTooLargeException::what() const throw()
{
	return "Payload Too Large Exception";
}

const char*	ConflictException::what() const throw()
{
	return "Conflict Exception";
}

const char*	RequestTimeoutException::what() const throw()
{
	return "Request Timeout Exception";
}

const char*	UnauthorizedException::what() const throw()
{
	return "Unauthorized Exception";
}
