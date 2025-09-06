#ifndef USER_HPP
#define USER_HPP

#include <string>

class User
{
public:
	User(const std::string &_username, const std::string &_password);
	bool	operator==(const User &other) const;
	const std::string& getUsername() const;

private:
	std::string username;
	std::string password;
};

#endif
