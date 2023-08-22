#include "../inc/Server.hpp"
#include "../inc/Client.hpp"
// #include "../inc/Messages.hpp"
#include "../inc/Channel.hpp"

Channel::Channel(){
	_channelName = "";
	_topic = "";
	_topicOperatorsOnly = false;
	_isLimitSet = false;
}

Channel::Channel(std::string name): _channelName(name) {
	_topic = "";
	_topicOperatorsOnly = false;
	_isLimitSet = false;
}

Channel::Channel(Channel const &cpy){
	*this = cpy;
}

Channel &Channel::operator=(Channel const &rhs){
	_channelName = rhs._channelName;
	_members = rhs._members;
	_operators = rhs._operators;
	_topic = rhs._topic;
	_topicOperatorsOnly = rhs._topicOperatorsOnly;
	_isLimitSet = rhs._isLimitSet;
	_nbLimit = rhs._nbLimit;

	return (*this);
}

Channel::~Channel(){

}

/*---------------------------------------------------------------------------------------------*/
// GETTERS
/*---------------------------------------------------------------------------------------------*/
std::string	Channel::getChannelName()
{
	return (_channelName);
}

std::string	Channel::getTopic()
{
	return (_topic);
}

std::string	Channel::getPassword()
{
	return _password;
}

std::vector<Client*>	Channel::getMember()
{
	return _members;
}

bool	Channel::getTopicMode()
{
	return _topicOperatorsOnly;
}

bool	Channel::getLimitMode()
{
	return _isLimitSet;
}

bool	Channel::getPassMode()
{
	return _isPasswordSet;
}

bool	Channel::getInviteMode()
{
	return _isInviteOnly;
}

int	Channel::getNbLimit()
{
	return _nbLimit;
}

std::string	Channel::getAllMembers()
{
	std::string	allMembers;

	for (std::vector<Client*>::iterator it=_members.begin(); it != _members.end(); it++)
	{
		allMembers += (*it)->getNick() + "@" + (*it)->getHostname() + " ";
	}
	// std::cout << "ALLMEMBERS = [" + allMembers + "]" << std::endl;
	return allMembers;
}

/*---------------------------------------------------------------------------------------------*/
// SETTERS
/*---------------------------------------------------------------------------------------------*/
void	Channel::setTopic(std::string topic, Client *client)
{
	if (_topicOperatorsOnly && (!isOperator(client)))
	{
		//Erreur t'as pas les droits
		return ;
	}
	_topic = topic;
}

void	Channel::setTopicMode(bool mode)
{
	_topicOperatorsOnly = mode;
}

void	Channel::setLimit(bool mode, int limit)
{
	_isLimitSet = mode;

	if (mode)
		_nbLimit = limit;
	else
		_nbLimit = -1;
	
	std::cout << "Limit = " << _nbLimit << std::endl;
}

bool	Channel::setOperator(bool mode, std::string username)
{
	for(std::vector<Client*>::iterator it=_members.begin(); it != _members.end(); it++)
	{
		if (mode)
		{
			if ((*it)->getNick() == username)
				return (addOperator(*it));
		}
		else
		{
			if ((*it)->getNick() == username)
				return (removeOperator(*it));
		}
	}
	return (false);
}

void	Channel::setPassMode(bool mode)
{
	_isPasswordSet = mode;
}

void	Channel::setChannelPassword(std::string password)
{
	_password = password;
}

void	Channel::setInviteMode(bool mode)
{
	_isInviteOnly = mode;
}

/*---------------------------------------------------------------------------------------------*/
// METHODS
/*---------------------------------------------------------------------------------------------*/
void Channel::addMember(Client *client)
{
	_members.push_back(client);
	return;
}

void Channel::addGuest(Client *client)
{
	_guests.push_back(client);
	return;
}

void	Channel::removeMember(Client *client, int fd)
{
	(void)client;
	for (std::vector<Client*>::iterator it=_members.begin(); it != _members.end(); it++)
	{
		if ((*it)->getFd() == fd)
		{
			_members.erase(it);
			return;
		}
	}
}

bool	Channel::isMember(Client *client)
{
	for (std::vector<Client*>::iterator it = _members.begin(); it != _members.end(); it++)
	{
		if ((*it)->getFd() == client->getFd())
			return true;
	}
	return false;
}

bool Channel::isMember(const std::string& nickname) {
	for (std::vector<Client*>::iterator it = _members.begin(); it != _members.end(); it++) {
		if ((*it)->getNick() == nickname)
			return true;
	}
	return false;
}

bool	Channel::addOperator(Client *client)
{
	bool		isMember = false;
	std::string	msg;

	for (std::vector<Client*>::iterator it = _members.begin(); it != _members.end(); it++)
	{
		if (client->getFd() == (*it)->getFd())
		{
			isMember = true;
			break ;
		}
	}

	if (isMember)
	{
		for (std::vector<Client*>::iterator itc = _operators.begin(); itc != _operators.end(); itc++)
		{
			if (client->getFd() == (*itc)->getFd())
			{
				std::cout << "already operator :" + (*itc)->getNick() << std::endl; // erreur existe deja ou osef
				return (false);
			}
		}
		_operators.push_back(client);
		return (true);
	}
	// pas un membre donc erreur
	std::cout << "Nice try, not a member : " + client->getNick() << std::endl; // erreur existe deja ou osef
	return (false);
}

bool	Channel::removeOperator(Client *client)
{
	bool		isMember = false;
	std::string	msg;

	for (std::vector<Client*>::iterator it = _members.begin(); it != _members.end(); it++)
	{
		if (client->getFd() == (*it)->getFd())
		{
			isMember = true;
			break ;
		}
	}

	if (isMember)
	{
		for (std::vector<Client*>::iterator itc = _operators.begin(); itc != _operators.end(); itc++)
		{
			if (client->getFd() == (*itc)->getFd())
			{
				// if (_operators.size() == 1)
				// {
				// 	//dernier operator dans le channel, on refuse
				// 	std::cout << "il faut un admin ici, tu restes." << std::endl;
				// 	return (false);
				// }
				std::cout << "operator removed :" + (*itc)->getNick() << std::endl; // erreur existe deja ou osef
				_operators.erase(itc);
				return (true);
			}
		}
	}
	// pas un membre donc erreur
	std::cout << "Nice try, not a member : " + client->getNick() << std::endl; // erreur existe deja ou osef
	return (false);

}

bool		Channel::isOperator(Client *client)
{
	for (std::vector<Client*>::iterator it = _operators.begin(); it != _operators.end(); it++)
	{
		if ((*it)->getFd() == client->getFd())
			return true;
	}
	return false;
}

bool	Channel::isNumber(std::string arg)
{
	for (unsigned int i = 0; i < arg.length(); i++)
	{
		if (!isdigit(arg[i]))
			return false;
	}
	return true;
}

bool		Channel::isNickMembre(std::string nickname)
{
	for (std::vector<Client*>::iterator it = _members.begin(); it != _members.end(); it++)
	{
		if ((*it)->getNick() == nickname)
			return true;
	}
	return false;
}


bool	Channel::isGuest(Client *client)
{
	for (std::vector<Client*>::iterator it = _guests.begin(); it != _guests.end(); it++)
	{
		if ((*it)->getFd() == client->getFd())
			return true;
	}

	return false;
}

void	Channel::removeGuest(Client *client)
{
	for(std::vector<Client*>::iterator it = _guests.begin(); it != _guests.end(); it++)
	{
		if ((*it)->getFd() == client->getFd())
		{
			_guests.erase(it);
			break;
		}
	}
}