#ifndef LOCATION_HPP
# define LOCATION_HPP
#include <sstream>
# include <vector>
# include <iostream>
# include <string>

class Location
{
	private:
	std::vector<std::string>	methods;
	std::string					alias;
	bool						autoindex;
	std::string					index;
	bool						is_redirect;
	std::string					redirect_target;
	int							redirect_code;

	void	parse_route_alias(std::istringstream& iss);
	void	parse_route_autoindex(std::istringstream& iss);
	void	parse_route_index(std::istringstream& iss);
	void	parse_route_redirect(std::istringstream& iss);


	public:
	Location();
	bool	parse_route_attributes(const std::string& line);
	void	parse_route_methods(std::istringstream& iss);
	void	addMethod(const std::string& method);
	void	setAlias(const std::string& _alias);
	void	enableAutoIndex();
	void	setIndex(const std::string& index);
	void	setRedirect(const std::string &target, int code);
	bool	isMethod(std::string const &method) const;
	bool	isAutoindexOn() const;
	const std::string	&getAlias() const;
	const std::string	&getIndex() const;
	const std::vector<std::string>& getMethods() const;
	bool	getIsRedirect() const;
	const std::string&	getRedirectTarget() const;
	int	getRedirectCode() const;
};

#endif
