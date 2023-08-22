#include "../inc/Client.hpp"
// #include "Server.hpp"

Client::Client():	_fd(0),
					_connected(false)
{
		_username = "";
		_nick = "";
		// _channel= "";
		_hostname= "";
		_isAuthenticated = false;
		nickSet = false;
}

Client::Client(int fd):	nickSet(false),
						_fd(fd),
						_connected(false)
{

}

Client::Client(Client const &src)
{
	*this = src;
}

Client	&Client::operator=(Client const &rhs)
{
	nickSet = rhs.nickSet;
	_fd = rhs._fd;
	_nick = rhs._nick;
	_username = rhs._username;
	_fullname = rhs._fullname;
	// _channel = rhs._channel;//utilisé ?
	_connected = rhs._connected;
	_hostname = rhs._hostname;
	_isAuthenticated = rhs._isAuthenticated;

	return (*this);
}

Client::~Client()
{

}

int		Client::getFd() const
{
	return (_fd);
}

std::string	Client::getNick()
{
	return _nick;
}

void	Client::setNick(std::string nick) {
	_nick = nick;
}

void		Client::setUser(std::string user) {
			_username = user;
		}

std::string	Client::getUser() {
			return _username;
		}

std::string Client::getHostname() {
	char hostname[256];
	if (gethostname(hostname, sizeof(hostname)) == 0) {
		return std::string(hostname);
	}
	return "";
}

bool Client::isAuthenticated(){
	return _isAuthenticated;
}

void Client::setIsAuthenticated(bool authenticated) {
		_isAuthenticated = authenticated;
}

bool Client::isConnected(){
	return _connected;
}