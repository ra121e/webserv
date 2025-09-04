#include "../include/User.hpp"

User::User(const std::string &_username, const std::string &_password)
	: username(_username), password(_password)
{
}

bool User::operator==(const User &other) const
{
	return username == other.username && password == other.password;
}
