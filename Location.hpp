#ifndef LOCATION_HPP
# define LOCATION_HPP
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
	int						redirect_code;
	
	public:
	Location();
	~Location();
	void	addMethod(const std::string& method);
	void	setAlias(const std::string& _alias);
	void	enableAutoIndex();
	void	setIndex(const std::string& index);
	void	setRedirect(const std::string &target, int code);

	bool	isMethod(std::string const &method) const;
	bool	isAutoindexOn() const;
	const std::string	&getAlias() const;
	const std::string	&getIndex() const;
	bool	getIsRedirect() const;
	std::string getRedirectTarget() const;
	int	getRedirectCode() const;
};

#endif
