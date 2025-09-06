#ifndef EXCEPTIONS_HPP
#define EXCEPTIONS_HPP
#include <exception>

class ResourceNotFoundException : public std::exception
{
	public:
		virtual const char* what() const throw();
};

class MethodNotAllowedException : public std::exception
{
	public:
		virtual const char* what() const throw();
};

class InternalServerErrorException : public std::exception
{
	public:
		virtual const char* what() const throw();
};

class BadRequestException : public std::exception
{
	public:
		virtual const char* what() const throw();
};

class PayloadTooLargeException : public std::exception
{
	public:
		virtual const char* what() const throw();
};

class ConflictException : public std::exception
{
	public:
		virtual const char* what() const throw();
};

class RequestTimeoutException : public std::exception
{
	public:
		virtual const char* what() const throw();
};

class UnauthorizedException : public std::exception
{
	public:
		virtual const char* what() const throw();
};

#endif
