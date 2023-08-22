#include "../inc/Server.hpp"
#include "../inc/Client.hpp"
#include "../inc/Messages.hpp"


Server::Server():	passIsValid(false),
					_socket(0),
					_validity(0),
					_port(0)
					// _quit(false)
{

}

Server::Server(Server const &src)
{
	*this = src;
}

Server	&Server::operator=(Server const &rhs)
{
	token = rhs.token;
	command = rhs.command;
	nickSet = rhs.nickSet;
	passIsValid = rhs.passIsValid;

	_socket = rhs._socket;
	_validity = rhs._validity;
	_port = rhs._port;
	_addr = rhs._addr;
	_sockets = rhs._sockets;
	_pfds = rhs._pfds;
	_clients = rhs._clients;
	_channels = rhs._channels;
	// _quit = rhs._quit;
	_password = rhs._password;
	_lastPing = rhs._lastPing;
	// _tokens = rhs._tokens;

	return (*this);
}

Server::~Server()
{
	if (currentClient != NULL)
	{
		delete currentClient;
		currentClient = NULL;
	}

	if (currentChannel != NULL)
	{
		delete currentChannel;
		currentChannel = NULL;
	}
}

void	Server::setPort(int port)
{
	_port = port;
}

int		Server::getPort()
{
	return (_port);
}

bool	Server::createSocket()
{
	int					option = 1;
	int					validity;

	_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (_socket == ERROR)
		throw (Server::ServException(ERRMSG"socket stream"));

	validity = setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
	if (validity == ERROR)
		throw (Server::ServException(ERRMSG"unable to free the socket"));

	validity = fcntl(_socket, F_SETFL, O_NONBLOCK);
	if (_socket == ERROR)
		throw (Server::ServException(ERRMSG"socket"));

	_sockets.push_back(_socket); // optionnel, peut-etre que le vecteur des sockets sera inutile

	bzero(&_addr, sizeof(_addr));
	_addr.sin_family = AF_INET;
	_addr.sin_addr.s_addr = INADDR_ANY;
	_addr.sin_port = htons(_port);

	validity = bind(_socket, (struct sockaddr *)&_addr, sizeof(_addr));
	if (validity == ERROR)
		throw (Server::ServException(ERRMSG"binding failed")); // binding failed

	validity = listen(_socket, BACKLOG);
	if (validity == ERROR)
		throw (Server::ServException(ERRMSG "validity listen failed")); //listen failed

	return (true);
}

void	Server::allSockets()
{
	if (_sockets.empty())
	{
		std::cout << "No live socket at the moment." << std::endl; // Change this message with errno
		return;
	}

	// DEBUG ONLY ///////////////////////////////////////////////////////////////////////
	std::cout << "Number of sockets: " << _sockets.size() << std::endl;

	std::cout << "Socket(s): | ";
	for (std::vector<int>::iterator	it = _sockets.begin(); it != _sockets.end(); it++)
		std::cout << (*it) << " | ";
	std::cout << std::endl;
	/////////////////////////////////////////////////////////////////////////////////////
}

int	Server::getSocket()
{
	return (_socket);
}

bool	Server::connection()
{
	pollfd		pfd;
	int			clientSock;
	int			pollCounter;
	char		buf[250];
	socklen_t	addrlen;
	std::map<int, std::string>	msgBuf;

	currentChannel = NULL;

	bzero(&pfd, sizeof(pfd));

	/* *********************************************************
	 * The first fd will be the one from the server.
	 * The subsequent ones, will be from the client(s).
	 * ********************************************************* */
	pfd.fd = _socket;
	pfd.events = POLLIN;
	_pfds.push_back(pfd);

	while (true)
	{
		pollCounter = poll(_pfds.data(), _pfds.size(), TIMEOUT_NO_P);

		if (pollCounter == ERROR)
			std::cout << ERRMSG << strerror(errno) << std::endl;

		for (unsigned int i = 0; i < _pfds.size(); i++)
		{
			if (_pfds[i].revents & POLLIN)
			{
				if (_pfds[i].fd == _socket)
				{
					addrlen = sizeof(_addr);
					clientSock = accept(_socket, (struct sockaddr *)&_addr, &addrlen);

					if (clientSock != ERROR)
					{
						pfd.fd = clientSock;
						pfd.events = POLLIN;
						_pfds.push_back(pfd);

						// New client
						currentClient = addClient(clientSock); // new way to handle client via Client class
						std::cout << "Bonjour, " << inet_ntoa(_addr.sin_addr) << ":" << ntohs(_addr.sin_port) << std::endl;
						capOrNOt(currentClient); // CAP LS NICK USER
						// getPing(buf, currentClient->getFd()); // PING PONG
					}
					else
						std::cout << ERRMSG << strerror(errno) << std::endl;
				}
				else
				{
					bzero(&buf, sizeof(buf));

					int	bytesNbr = recv(_pfds[i].fd, buf, sizeof(buf), 0);
					for (std::vector<Client*>::iterator	it = _clients.begin(); it != _clients.end(); it++)
					{
						if ((*it)->getFd() == _pfds[i].fd)
						{
							currentClient = (*it);
							break;
						}
					}
					int	sender = _pfds[i].fd;
					msgBuf[sender] += static_cast<std::string>(buf);
					std::string	rawMsgWithoutEndSeq = msgBuf[sender].substr(0, msgBuf[sender].find(END_SEQUENCE));

					currentChannel = getCurrentChannel(rawMsgWithoutEndSeq);

					if (static_cast<std::string>(buf).find("\n") != std::string::npos)
					{
						inputClient(msgBuf[sender], currentClient, currentChannel);
						msgBuf[sender].clear();
					}

					//bzero(&buf, 250);

					if (bytesNbr <= 0)
					{
						if (bytesNbr == 0)
						{
							std::cout << "socket " << sender << " is gone." << std::endl;
							// Quand il n'y a plus de client, juste le serveur
							// if (_pfds.size() == 2)
								// std::cout << _pfds.size() << " * cricket noises *" << std::endl;
						}
						else
							std::cout << ERRMSG << strerror(errno) << std::endl;

						for (std::vector<Client*>::iterator	it = _clients.begin(); it != _clients.end(); it++)
						{
							if ((*it)->getFd() == _pfds[i].fd)
							{
								for (std::vector<Channel*>::iterator itc = _channels.begin(); itc != _channels.end(); itc++)
								{
									if ((*itc)->isMember(*it))
										(*itc)->removeMember((*it), (*it)->getFd());

									if ((*itc)->isOperator(*it))
										((*itc)->removeOperator(*it));

									if ((*itc)->isGuest(*it))
										((*itc)->removeGuest(*it));
								}
								delete (*it);
								_clients.erase(it);
								break;
							}
						}
						msgBuf[_pfds[i].fd].clear();
						close(_pfds[i].fd);
						_pfds.erase(_pfds.begin() + i);
						i--;
					}
					else
					{
						continue;
						// for (std::vector<pollfd>::iterator it = _pfds.begin(); it != _pfds.end(); it++)
						// {
						// 	int	dest = (*it).fd;

						// 	if (dest != _socket && dest != sender)
						// 	{
						// 		// VERIFIER QUE CE N'EST PAS UNE COMMANDE MAIS DU TEXTE A ENVOYER
						// 		if (send(dest, buf, bytesNbr, 0) == ERROR)
						// 			std::cout << ERRMSG << strerror(errno) << (*it).fd << std::endl;
						// 		// else
						// 		// 	sendMsg(buf, dest);//debug ONLY !!! to erase
						// 	}
						// }
					}
				}
			}
		}
	}
	// getPing(buf, currentClient->getFd()); // PING PONG
}

void Server::parseCommand(std::string buf)
{
	std::string input(buf);
	size_t spacePos = input.find(' ');

	if (spacePos != std::string::npos)
	{
		token = input.substr(0, spacePos);
		command = input.substr(spacePos + 1, input.size());
	}
	else
	{
		token = input;
		command.clear();
	}

	// Affichage des résultats
	std::cout << "Token: " << token << std::endl;
	std::cout << "Command: " << command << std::endl;
	std::cout << std::endl;
}


void	Server::inputClient(std::string buf, Client *client, Channel *channel)
{
	size_t pos = buf.find(END_SEQUENCE);

	while (pos != std::string::npos)
	{
		std::string line = buf.substr(0, pos);
		buf.erase(0, pos + 2);
		pos = buf.find(END_SEQUENCE);

		std::cout << "RECEIVED : " << line << std::endl;
		if (! line.empty()) {
			parseCommand(line);
			commands(token, client, channel);
		}
		else
			sendErrorMsg(461, client->getFd(), "", "", "", "");
	}
}

Client* Server::addClient(int fd)
{
	Client* client = new Client(fd); // Allouer dynamiquement un nouvel objet Client
	_clients.push_back(client);
	return client;
}

Channel* Server::addChannel(std::string name)
{
	Channel* channel = new Channel(name); // Allouer dynamiquement un nouvel objet Channel
	_channels.push_back(channel);
	return channel;
}

void	Server::removeChannel(Channel *channel)
{
	for (std::vector<Channel*>::iterator it=_channels.begin(); it != _channels.end(); it++)
	{
		if (channel->getChannelName() == (*it)->getChannelName())
		{
			_channels.erase(it);
			break;
		}
	}
}

std::string	Server::parseChan(std::string buf, size_t pos)
{
	std::string	chanName;
	size_t		spacePos;
	size_t		commaPos;
	size_t		endPos;
	
	if (pos == 0)
		pos = buf.find("#");

	if (pos != std::string::npos && pos != buf.size() - 1)
	{
		spacePos = buf.find(" ", pos);
		commaPos = buf.find(",", pos);

		endPos = std::min(spacePos, commaPos);
		if (endPos == std::string::npos)
			endPos = buf.size();
		chanName = buf.substr(pos, endPos - pos);
	}
	else
		chanName = buf;
	return chanName;
}

std::vector<std::string>	Server::parseModeCmd(std::string buf)
{
	std::vector<std::string>	modeCmds;
	size_t						firstPos;
	size_t						lastPos;
	bool						isAdded;
	bool						validArg;
	std::string					instruction = "";
	std::map<int, char>			argsByMode;

	argsByMode['i'] = 0;
	argsByMode['t'] = 0;
	argsByMode['k'] = 1;
	argsByMode['o'] = 1;
	argsByMode['l'] = 1;

	firstPos = buf.find_first_of("+-");
	lastPos = buf.find(" ", firstPos);
	modeCmds.clear();

	if (firstPos != std::string::npos && lastPos != std::string::npos)
		buf = buf.substr(firstPos, lastPos - firstPos);
	else if (firstPos != std::string::npos)
		buf = buf.substr(firstPos);
	else
		return modeCmds;

	for (size_t i=0; i < buf.size(); i++)
	{
		
		if (buf[i] == '+')
			isAdded = true;
		else if (buf[i] == '-')
			isAdded = false;
		else
		{
			validArg = false;
			for (std::map<int, char>::iterator it = argsByMode.begin(); it != argsByMode.end(); it++)
			{
				if (buf[i] == (*it).first)
				{
					validArg = true;
					break ;
				}
			}

			if (!validArg)
			{
				modeCmds.clear();
				return modeCmds;
			}

			if (isAdded)
			{
				instruction = "+";
				instruction += buf[i];
				modeCmds.push_back(instruction);
			}
			else
			{
				instruction = "-";
				instruction += buf[i];
				modeCmds.push_back(instruction);
			}
		}
	}
	return modeCmds;
}

std::string	Server::modeTargetMember(std::string buf)
{
	size_t	lastParamPos;

	lastParamPos = buf.find_last_of(" ");

	if (lastParamPos != std::string::npos)
		return(buf.substr(lastParamPos));
	return "";
}

Channel*	Server::getCurrentChannel(std::string msgBuf)
{
	std::string		chanName;

	chanName = parseChan(msgBuf, 0);

	for(std::vector<Channel*>::iterator it=_channels.begin(); it != _channels.end(); it++)
	{
		if ((*it)->getChannelName() == chanName)
			return (*it);
	}
	return (NULL);
}

std::string	Server::getPassword() {
	return _password;
} //@Verena to print the password entered

void	Server::setPassword(std::string pass)  {
	_password = pass;
}

 void	Server::capOrNOt(Client *client) {
	std::string version = "2.0";
	std::string channel = "#mychannel";

	//print clientNumber (socket)
	std::cout << "NEW CLIENT : " << client->getFd() << std::endl; // check and debug only

	// // Connexion au serveur
	if (connect(client->getFd(), (struct sockaddr*)&_addr, sizeof(_addr)) < 0) {
		std::cerr <<  "other connection detected"  << std::endl;
		sendMsg("PING", client->getFd()); // a voir si utile et necessaire
		return;
	}
}

bool		Server::channelExists(std::string channel)
{
	for (std::vector<Channel*>::iterator it = _channels.begin(); it != _channels.end(); it++)
	{
		if ((*it)->getChannelName() == channel)
			return true;
	}
	return false;
}

bool	Server::isNickUsed(std::string nick)
{
	for (std::vector<Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
		if ((*it)->getNick() == nick) 
		{
			return true;
		}
	}
	return false;
}

// std::vector<std::string> Server::getToken()
// {
// 	return _tokens;
// }

// void Server::addToken(std::string token)
// {
// 	_tokens.push_back(token);
// 	return;
// }