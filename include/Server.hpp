#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <sys/types.h>
#include <vector>
#include <map>
#include "HttpRequest.hpp"
#include <fstream>
#include <sstream>
#include "Location.hpp"
#include "Network.hpp"
#include "../include/User.hpp"
#include "SharedPointer.hpp"

class Server
{
private:
	std::vector<SharedPointer<Network> >				networks;
	uint64_t							client_max_body_size;
	std::map<std::string, std::string>	error_pages;
	std::map<std::string, Location>		locations;
	std::vector<User>					users;
	std::vector<std::string>			session_ids;
	
public:
	void	parse_listen(std::istringstream &ss);
	void	parse_client_max_body_size(std::istringstream &ss);
	void	parse_error_pages(std::ifstream& infile, std::istringstream& ss);
	bool	parse_single_error_page(const std::string& line);
	void	parse_route(std::ifstream& infile, std::istringstream& ss);
	void	addNetwork(const SharedPointer<Network>& net);
	void	setClientMaxBodySize(uint64_t _client_max_body_size);
	void	addErrorPage(const std::string& error, const std::string& page);
	void	addLocation(const std::string& path, const Location& location);
	std::map<std::string, Location>::const_iterator
	getLocationIteratorMatch(std::string const &uri, const std::string& extension,
		HttpRequest& request) const;
	const std::map<std::string, Location> &getLocations() const;
	void	setup();
	const std::vector<SharedPointer<Network> >&	getNetworks() const;
	std::string getErrorPage(int code) const;
	uint64_t getClientMaxBodySize() const;
	bool	addUser(const std::string& username, const std::string& password);
	bool	authenticateUser(const std::string& username, const std::string& password) const;
	void	addSessionId(const std::string& session_id);
	bool	isValidSessionId(const std::string& session_id) const;
	void	removeSessionId(const std::string& session_id);
};

#endif
