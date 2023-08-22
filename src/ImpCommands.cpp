#include "../inc/Server.hpp"
#include "../inc/Client.hpp"
#include "../inc/Messages.hpp"
#include "../inc/Channel.hpp"


void	Server::commands(std::string cmd, Client *client, Channel *channel) {

	std::string _cmdArray[CMDNBR] = {"CAP", "PING", "NICK", "USER", "JOIN", "MODE", "PRIVMSG", "NOTICE", "TOPIC", "PART", "KICK", "INVITE", "PASS", "QUIT"};

	void	(Server::*functionPtr[])(Client *client, Channel *channel) = {
		&Server::CAP,
		&Server::PING,
		&Server::NICK,
		&Server::USER,
		&Server::JOIN,
		&Server::MODE,
		&Server::PRIVMSG,
		&Server::NOTICE,
		&Server::TOPIC,
		&Server::PART,
		&Server::KICK,
		&Server::INVITE,
		&Server::PASS,
		&Server::QUIT
	};

	for (int i = 0; i < CMDNBR; i++) {
		if (cmd.compare(_cmdArray[i]) == 0) {
			(this->*functionPtr[i])(client, channel);
			return;
		}
	}
}

void	Server::CAP(Client *client, Channel *channel) {
	(void)channel;
	if (command[0] == 'L' && command[1] == 'S')
		sendMsg("CAP * LS :", client->getFd());
	else
		return;
}

void	Server::PING(Client *client, Channel *channel) {
	(void)channel;
	std::string ping = token;
	std::string pingContent = command;
	// Construire la réponse PONG avec le même contenu que le message PING
	std::string pongResponse = "PONG " + pingContent;
	// _lastPing = time(NULL);
	// Envoyer la réponse PONG au client
	sendMsg(pongResponse, client->getFd());
}

void Server::NICK(Client *client, Channel *channel) {
	(void)channel;

	// if (passIsValid == false) {
	// 	sendErrorMsg(ERR_NEEDMOREPARAMS, client->getFd(), "", "PASSWORD REQUIERED", "", "");
	// 	exit(EXIT_FAILURE);
	// }

	if (client->isAuthenticated() == false) {
		// sendErrorMsg(ERR_NEEDMOREPARAMS, client->getFd(), client->getNick(), "PASSWORD REQUIERED", "", "");
		// sendErrorMsg(ERR_PASSWDMISMATCH, client->getFd(),"", "", "", "");
		// exit(EXIT_FAILURE);
		return;
	}

	if (client->nickSet == false) {
		std::string nickname = command;
		int numberFd = client->getFd();
		std::stringstream ss;
		ss << numberFd;
		std::string strNumberFd = ss.str();
		nickname += strNumberFd;
		client->setNick(nickname);
		std::string msg = ":" + command + " NICK " + nickname;
		sendMsg(msg, client->getFd());
		client->nickSet = true;
	}
	else {
		std::string newNick = command;

		// vérifie si le nouveau surnom dépasse 30 caractères
		if(newNick.size() > 30) {
		sendErrorMsg(ERR_ERRONEUSNICKNAME, client->getFd(), newNick, "", "", "");
		std::cerr << "Error: Nickname is longer than 30 characters." << std::endl; //comme dans freenode
		return;
	}
	// vérifie si le surnom existe déjà
	for (std::vector<Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
		if ((*it)->getNick() == newNick) {
			sendErrorMsg(ERR_NICKNAMEINUSE, client->getFd(), newNick, "", "", "");
			return;
		}
	}
	// vérifie si le nouveau surnom respecte les règles
	if (newNick.empty() || newNick[0] == '#' || newNick[0] == ':' || newNick.find_first_of(CHANTYPES) != std::string::npos || newNick.find(' ') != std::string::npos) {
		sendErrorMsg(ERR_ERRONEUSNICKNAME, client->getFd(), newNick, "", "", "");
		return ;
	}
		// continue avec le reste du code si les conditions sont remplies
		for (std::vector<Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
			if ((*it)->getFd() == client->getFd()) {
				std::string oldNick;
				for (std::vector<Client*>::iterator innerIt = _clients.begin(); innerIt != _clients.end(); ++innerIt) {
					if ((*innerIt)->getFd() == client->getFd()) {
						oldNick = (*innerIt)->getNick();
						break;
					}
				}

				(*it)->setNick(newNick);

				std::string msg;

				msg = ":" + oldNick + " NICK " + newNick;
				sendMsg(msg, client->getFd());
				break;
			}
		}
	}
}

void	Server::USER(Client *client, Channel *channel) {
	(void)channel;

	// if (std::find(_tokens.begin(), _tokens.end(), "PASS") == _tokens.end() && client->isAuthenticated() == false) {
	// 	// "PASS" is not in _tokens
	// 	sendErrorMsg(ERR_NEEDMOREPARAMS, client->getFd(), "", "PASSWORD REQUIERED", "", "");
	// 	return;
	// }

	if (client->isAuthenticated() == false) {
		// sendErrorMsg(ERR_NEEDMOREPARAMS, client->getFd(), client->getNick(), "PASSWORD REQUIERED", "", "");
		// sendErrorMsg(ERR_PASSWDMISMATCH, client->getFd(),"", "", "", "");
		return;
	}
	// if (command.empty())
	// 	sendErrorMsg(ERR_NEEDMOREPARAMS, client->getFd(), client->getNick(), "", "", ""); on verra si c est necessaire
	std::size_t colonPos = command.find(':');
	if (colonPos != std::string::npos) {
		std::string UserContent = command.substr(colonPos + 1);
		std::string	msg = "USER : " + UserContent + END_SEQUENCE;
		client->setUser(UserContent);
		// if (client->isAuthenticated() && !client->getUser().empty() && !client->getNick().empty())
		first_message(client);
		}
	}

void	Server::JOIN(Client *client, Channel *channel) {
	(void)channel;
	std::cout << "cmd join" << std::endl;

	bool						channelExists = false;
	std::string					chanName;
	size_t						pos = 0;
	size_t						hashtagPos = 0;
	std::vector<std::string>	channelsToAdd;
	std::string					msg;
	std::string					passwordEntered;
	
	if (command == ":")
		return;

	while ((hashtagPos = command.find("#", hashtagPos)) != std::string::npos)
	{
		chanName = parseChan(command, hashtagPos);

		if (chanName[0] != '#')
			chanName = '#' + chanName;
		
		channelsToAdd.push_back(chanName);

		hashtagPos = hashtagPos + 1;
	}

	if (channelsToAdd.empty())
	{
		sendErrorMsg(ERR_NEEDMOREPARAMS, client->getFd(), client->getNick(), "JOIN", "Not enough parameters", "");
		return;
	}

	if ((pos = command.find(" ")) != std::string::npos)
	{
		if (command.find(":") == std::string::npos)
			passwordEntered = command.substr(pos + 1);
	}

	std::cout << ": [" + passwordEntered + "]" << std::endl;

	for (std::vector<std::string>::iterator itc = channelsToAdd.begin(); itc != channelsToAdd.end(); itc++)
	{
		for (std::vector<Channel*>::iterator	it = _channels.begin(); it != _channels.end(); it++)
		{
			if (((*it)->getChannelName() == (*itc)))
			{
				channelExists = true;

				if (channel->getLimitMode())
				{
					if (static_cast<int>(channel->getMember().size()) >= channel->getNbLimit())
					{
						sendErrorMsg(ERR_CHANNELISFULL, client->getFd(), client->getNick(), channel->getChannelName(), "", "");
						return ;
					}
				}

				if (channel->getPassMode())
				{
					if (passwordEntered != channel->getPassword())
					{
						sendErrorMsg(ERR_BADCHANNELKEY, client->getFd(), client->getNick(), channel->getChannelName(), "", "");
						return ;
					}
				}

				if (channel->getInviteMode())
				{

					if (channel->isMember(client))
					{
						sendErrorMsg(ERR_USERONCHANNEL, client->getFd(), client->getNick(), channel->getChannelName(), "", "");
						return ;
					}

					if (!(channel->isGuest(client)))
					{
						sendErrorMsg(ERR_INVITEONLYCHAN, client->getFd(), channel->getChannelName(), client->getNick(), "", "");
						return ;
					}
					std::cout << "Oh, you're on our list. You may proceed." << std::endl;
				}

				std::cout << "Channel [" + (*itc) + "] already exist. You'll join 'em" << std::endl;
				currentChannel = (*it);
				currentChannel->addMember(client);
				break;
			}
		}

		if (!channelExists)
		{
			currentChannel = addChannel((*itc));
			std::cout << "Channel [" + (*itc) + "] created. You're a VIP now." << std::endl;
			currentChannel->addMember(client);
			currentChannel->addOperator(client);
		}

		pos = command.find(" :");
		if (pos != std::string::npos && (std::string::npos + 1) != (*itc).size())
			currentChannel->setTopic(command.substr(pos + 2, command.size()), client);

		// send info to client
		msg = ":" + client->getNick() + "@" + client->getHostname() + " JOIN " + (*itc);
		sendMsg(msg, client->getFd());

		// send info of all members in the channel
		// msg = ":" + client->getNick() + "@" + client->getHostname() + " = " + (*itc) + " :" + currentChannel->getAllMembers();
		// sendMsg(msg, client->getFd());

		if (!channelExists)
		{
			msg = "MODE " + (*itc) + " +o "+ client->getNick();
			sendMsg(msg, client->getFd());
		}

		// msg = ":" + (*itc) + " :End of /NAMES list.";
		// sendMsg(msg, client->getFd());

		msg = ":" + client->getNick() + "@" + client->getHostname() + " JOIN " + (*itc);
		sendMsgToAllMembers(msg, client->getFd());

		if (!currentChannel->getTopic().empty())
		{
			msg = "TOPIC " + (*itc) + " :" + currentChannel->getTopic();

			sendMsg(msg, client->getFd());
			sendMsgToAllMembers(msg, client->getFd());
		}
	}
}

void	Server::MODE(Client *client, Channel *channel) {
	std::cout << "cmd mode" << std::endl;
	std::string					msg = "";
	std::string					chanName;
	std::vector<std::string>	modesVec;
	std::vector<std::string>	args;
	size_t						pos = 0;
	size_t						endPos;
	bool						isAdded = false;

	std::cout << "command recue [" + command + "]" << std::endl;

	// lors de la connexion initiale
	if (command == (client->getNick() + " +i"))
		return;

	chanName = parseChan(command, 0);
	bool	isChannel = channelExists(chanName);

	if (!isChannel)
	{
		sendErrorMsg(ERR_NOSUCHCHANNEL, client->getFd(), client->getNick(), chanName, "", "");
		return ;
	}

	if (!channel->isOperator(client))
	{
		sendErrorMsg(ERR_CHANOPRIVSNEEDED, client->getFd(), client->getNick(), channel->getChannelName(), "Not allowed", "");
		return ;
	}

	pos = chanName.size() + 1;

	modesVec = parseModeCmd(command.substr(pos));
	if (modesVec.empty())
	{
		sendErrorMsg(ERR_UNKNOWNMODE, client->getFd(), "", "", "", "");
		return ;
	}
	
	std::string	tempura;

	while ((pos = command.find(" ", pos)) != std::string::npos && pos < command.size())
	{
		endPos = command.find(" ", (pos + 1));
		if (endPos == std::string::npos)
			endPos = command.size();
		tempura = command.substr((pos + 1), ((endPos - pos) - 1));
		args.push_back(tempura);
		pos = endPos;
	}

	if (args.size() > 3)
	{
		sendErrorMsg(ERR_UNKNOWNMODE, client->getFd(), client->getNick(), "", ":too many args", "");
		return;
	}

	pos = 0;
	for (std::vector<std::string>::iterator it = modesVec.begin(); it != modesVec.end(); it++)
	{
		if ((*it).find("+") != std::string::npos)
			isAdded = true;
		else
			isAdded = false;

		if ((*it).find("t") != std::string::npos)
		{
			channel->setTopicMode(isAdded);
			if (isAdded)
				msg = ":" + client->getNick() + " MODE " + channel->getChannelName() + " " + (*it) + " :Channel topic is restricted to operator(s)";
			else
				msg = ":" + client->getNick() + " MODE " + channel->getChannelName() + " " + (*it) + " :Channel topic can be set by everyone";	
		}
		else if ((*it).find("o") != std::string::npos)
		{
			if (args.size() == 0)
			{
				sendErrorMsg(ERR_NEEDMOREPARAMS, client->getFd(), channel->getChannelName(), "Invalid channel limit", "", "");
				return ;
			}

			if (channel->setOperator(isAdded, args.back()))
			{
				if (isAdded)
					msg = ":" + client->getNick() + " MODE " + channel->getChannelName() + " " + (*it) + " " + args.back() + " :has been granted operator status.";			
				else
					msg = ":" + client->getNick() + " MODE " + channel->getChannelName() + " " + (*it) + " " + args.back() + " :has been removed from operators";		
			}
		}
		else if ((*it).find("l") != std::string::npos)
		{
			if (args.size() == 0 && (isAdded))
			{
				sendErrorMsg(ERR_NEEDMOREPARAMS, client->getFd(), "", "", "", "");
				return ;
			}

			if (isAdded)
			{
				int limit = std::atoi(args.front().c_str());
				if (limit <= 0)
				{
					sendErrorMsg(ERR_UNKNOWNMODE, client->getFd(), "", "", "", "");
					return ;
				}
				channel->setLimit(isAdded, limit);
				msg = ":" + client->getNick() + " MODE " + channel->getChannelName() + " " + (*it) + " " + args.front() + " :Channel limit set to " + args.front();			
			}
			else
			{
				channel->setLimit(isAdded, 0);
				msg = ":" + client->getNick() + " MODE " + channel->getChannelName() + " " + (*it) + " :Channel limit removed";
			}
		}
		else if ((*it).find("k") != std::string::npos)
		{
			if (args.size() == 0 && (isAdded))
			{
				sendErrorMsg(ERR_NEEDMOREPARAMS, client->getFd(), "", "", "", "");
				return ;
			}
			
			if (isAdded)
			{
				std::string	password;
				if (args.size() == 1)
					password = args.front();
				else if (args.size() == 2)
				{
					if (channel->isNumber(args.front()))
						password = args[1];
					else
						password = args.front();
				}
				else
					password = args[1];

				channel->setChannelPassword(password);
			}

			channel->setPassMode(isAdded);
			if (isAdded)
				msg = ":" + client->getNick() + " MODE " + channel->getChannelName() + " " + (*it) + " :a password has been set for this channel";			
			else
				msg = ":" + client->getNick() + " MODE " + channel->getChannelName() + " " + (*it) + " :the channel password has been removed";
		}
		else if ((*it).find("i") != std::string::npos)
		{
			channel->setInviteMode(isAdded);

			if (isAdded)
				msg = ":" + client->getNick() + " MODE " + channel->getChannelName() + " " + (*it) + " :the channel is now on invitation-only";			
			else
				msg = ":" + client->getNick() + " MODE " + channel->getChannelName() + " " + (*it) + " :the channel is no longer on invitation-only";
		}
		else
		{
			sendErrorMsg(ERR_UNKNOWNMODE, client->getFd(), "", "", "", "");
			return;
		}

		if (!msg.empty())
		{
			sendMsg(msg, client->getFd());
			sendMsgToAllMembers(msg, client->getFd());
		}
	}
}

//for PRIVMSG
size_t countSubstring(const std::string& str, std::string& sub) {
	if (sub.length() == 0) 
		return 0;
	size_t count = 0;
	std::size_t lastChar = sub.find_last_not_of(' ');
	if (lastChar != std::string::npos) {
		sub = sub.substr(0, lastChar + 1);
	}
	for (size_t offset = str.find(sub); offset != std::string::npos;
		 offset = str.find(sub, offset + 1)) {
		count++;
	}
	return count;
}

void Server::PRIVMSG(Client* client, Channel* channel) {
	std::cout << "cmd privmsg" << std::endl;
	bool	msgSend = false;
	size_t parsePoint = command.find(':');
	std::string channelName = command.substr(1, parsePoint - 1);  // Get the channel name
	std::string messChan = command.substr(parsePoint + 1);;  // Get the message
	channelName = '#' + channelName;
	// Check how many times the channel name appears in the command
	std::size_t count = countSubstring(command, channelName);
	if (count > 1) {
		std::cout << "messCHan :" << messChan << std::endl;
		
		std::string msg = ':' + client->getNick() + '@' + client->getHostname() + " " + token + " " + channelName + " :" + messChan;
		sendMsgToAllMembers(msg, client->getFd());
		msgSend = true;
	}
	if (command.find('#') != std::string::npos && count == 1 && msgSend == false) {
		std::vector<std::string> hashChan;
		std::string allChanMsg;

		// Séparer la commande en mots à l'aide de la fonction parseChan
		size_t pos = command.find("#");
		while (pos != std::string::npos) {
			std::string chanName = parseChan(command, pos);
			hashChan.push_back(chanName);
			pos = command.find('#', pos + 1);
		}
		if (!hashChan.empty()) {
			std::string lastHashWord = hashChan.back();
			std::size_t lastHashPos = command.rfind(lastHashWord);
			if (lastHashPos != std::string::npos) {
				allChanMsg = command.substr(lastHashPos + lastHashWord.size() + 2); // soucis de parsing a verifier
			}
		}
		for (size_t i = 0; i < hashChan.size(); i++) {
			std::cout << "mot avec # : " << hashChan[i] << std::endl;
			std::cout << "message a envoyer : " << allChanMsg << std::endl;

			bool found = false;

			// Comparaison avec les noms de channels existants
			for (size_t j = 0; j < _channels.size(); ++j) {
				if (_channels[j]->getChannelName() == hashChan[i]) {
					// Check if the client is a member of the channel
					if (_channels[j]->isMember(client)) {
						found = true;
						// Send the message to all members of the corresponding channel
						std::string msg = ':' + client->getNick() + '@' + client->getHostname() + " " + token + " " + hashChan[i] + " :" + allChanMsg;
						sendMsgToAllMembers(msg, client->getFd());
						return;
					} else {
						// The client is not a member of the channel
						sendErrorMsg(ERR_CANNOTSENDTOCHAN, client->getFd(), channel->getChannelName(), "", "", "");
						return;
					}
				}
			}
			if (!found) {
				sendErrorMsg(ERR_NOSUCHCHANNEL, client->getFd(), hashChan[i], "", "", "");
				return;
			}
		}
	}
	else if (command.find('#') == std::string::npos) {
		std::size_t msgPos = command.find(":");
		if (msgPos != std::string::npos) {
			std::string privMsg = command.substr(msgPos + 1);

			// Recherche de l'indice du premier espace avant le ':'
			std::size_t nickPos = command.find(" ");
			if (nickPos != std::string::npos) {
				// Extraction du nickname
				std::string nickname = command.substr(0, nickPos);
				std::cout << "nickname [" << nickname << "]" << std::endl;
				std::cout << "client [" << client->getNick() << "]" << std::endl;
				// Find the recipient client from _clients vector
				// Client* recipientClient = NULL;
				for (std::vector<Client*>::iterator it = _clients.begin(); it != _clients.end(); it++)
				{
					if (nickname == (*it)->getNick())
					{ // || (*it)->getNick() == client->getNick()) {
						std::cout << "nickname2 [" << nickname << "]" << std::endl;
						std::cout << "client2 [" << client->getNick() << "]" << std::endl;
						std::cout << "it [" << (*it)->getNick() << "]" << std::endl;
					
					//	std::string privMsgNick = ":" + (*it)->getNick() + "!~" + client->getUser() + "@" + client->getHostname() + " " + token + " " + client->getNick() + " :" + privMsg;
						std::string privMsgNick = ":" + client->getNick() + " " + token + " " + (*it)->getNick() + " :" + privMsg;
						std::cout << "ca rentre dedant et le nick d envoi est : " << client->getNick() << std::endl;
						std::cout << "Trying to send message to: " << (*it)->getNick() << std::endl;
						sendMsg(privMsgNick, (*it)->getFd());
						return ;
					}
				}
				sendErrorMsg(ERR_NOSUCHNICK, client->getFd(), client->getNick(), "", "", "");		
				return ;
				}
			}
		} 
	sendErrorMsg(ERR_CANNOTSENDTOCHAN, client->getFd(), channel->getChannelName(), "", "", "");
}


void Server::NOTICE(Client *client, Channel *channel) { 
	(void)channel;
	std::cout << "cmd notice" << std::endl;
	size_t parsePoint = command.find(':');
	std::string channelName = command.substr(1, parsePoint - 1);  // Get the channel name
	std::string messChan = command.substr(parsePoint + 1);;  // Get the message
	channelName = '#' + channelName;
	// Check how many times the channel name appears in the command
	std::size_t count = countSubstring(command, channelName);
	if (count > 1) {
		std::cout << "messCHan :" << messChan << std::endl;
		
		std::string msg = ':' + client->getNick() + '@' + client->getHostname() + " " + token + " " + channelName + " :" + messChan;
		sendMsgToAllMembers(msg, client->getFd());
	}
	if (command.find('#') != std::string::npos && count == 1) {
		std::vector<std::string> hashChan;
		std::string allChanMsg;

		// Séparer la commande en mots à l'aide de la fonction parseChan
		size_t pos = command.find("#");
		while (pos != std::string::npos) {
			std::string chanName = parseChan(command, pos);
			hashChan.push_back(chanName);
			pos = command.find('#', pos + 1);
		}
		if (!hashChan.empty()) {
			std::string lastHashWord = hashChan.back();
			std::size_t lastHashPos = command.rfind(lastHashWord);
			if (lastHashPos != std::string::npos) {
				allChanMsg = command.substr(lastHashPos + lastHashWord.size() + 2); // soucis de parsing a verifier
			}
		}
		for (size_t i = 0; i < hashChan.size(); i++) {
			std::cout << "mot avec # : " << hashChan[i] << std::endl;
			std::cout << "message a envoyer : " << allChanMsg << std::endl;

			// bool found = false;

			// Comparaison avec les noms de channels existants
			for (size_t j = 0; j < _channels.size(); ++j) {
				if (_channels[j]->getChannelName() == hashChan[i]) {
					// Check if the client is a member of the channel
					if (_channels[j]->isMember(client)) {
						// found = true;
						// Send the message to all members of the corresponding channel
						std::string msg = ':' + client->getNick() + '@' + client->getHostname() + " " + token + " " + hashChan[i] + " :" + allChanMsg;
						sendMsgToAllMembers(msg, client->getFd());
						break;
					} else {
						// The client is not a member of the channel
						sendErrorMsg(ERR_CANNOTSENDTOCHAN, client->getFd(), channel->getChannelName(), "", "", "");
					}
				}
			}
		}
	} else if (command.find('#') == std::string::npos) {
		std::size_t msgPos = command.find(":");
		if (msgPos != std::string::npos) {
			std::string privMsg = command.substr(msgPos + 1);

			// Recherche de l'indice du premier espace avant le ':'
			std::size_t nickPos = command.find(" ");
			if (nickPos != std::string::npos) {
				// Extraction du nickname
				std::string nickname = command.substr(0, nickPos);
				
				// Find the recipient client from _clients vector
				Client* recipientClient = NULL;
				for (std::vector<Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
					if ((*it)->getNick() == nickname) {
						recipientClient = *it;
						break;
					}
				}
				if (recipientClient) {
					std::string privMsgNick = "<" + client->getNick() + "> send you : " + privMsg;
					sendMsg(privMsgNick, recipientClient->getFd());
				}
			}
		}
	}
}

void	Server::TOPIC(Client *client, Channel *channel) {
	std::cout << "cmd topic" << std::endl;
	std::string		topicName;
	std::string		msg;
	size_t			pos = command.find(":");

	if (channel == NULL)
	{
		sendErrorMsg(ERR_NOSUCHCHANNEL, client->getFd(), "", "", "", "");
		return;
	}

	// le /topic seul est gere automatiquement sans passer par TOPIC
	if (command.find("::") != std::string::npos)
	{
		msg = ": TOPIC " + channel->getChannelName();
		channel->setTopic("", client);
	}
	else
	{
		if (pos != std::string::npos && (std::string::npos + 1) != command.size())
			topicName = command.substr(pos + 1);
		else
			topicName = command;

		std::cout << "commande ds topic : [" + command + "]" << std::endl;
		std::cout << "topicName ds topic : [" + topicName + "]" << std::endl;
		std::cout << "channel name ds topic : [" + channel->getChannelName() + "]" << std::endl;

		channel->setTopic(topicName, client);
		msg = ": 332 " + client->getNick() + " " + channel->getChannelName() + " :" + channel->getTopic();
	}

	sendMsg(msg, client->getFd());
	// lors d'un changement de sujet, envoyer a tous les membres via TOPIC qqch
	sendMsgToAllMembers(msg, client->getFd());

}

void	Server::PART(Client *client, Channel *channel){
	if (channel == NULL){
		std::cout << "channel null" << std::endl;
		sendErrorMsg(ERR_NOSUCHCHANNEL, client->getFd(), "", "", "", "");
		return;
	}

	if (channel->isGuest(client))
		(channel->removeGuest(client));

	if (channel->isOperator(client))
		(channel->removeOperator(client));

	if (channel->isMember(client))
		channel->removeMember((client), client->getFd());


	// channel->removeMember(client, client->getFd());
	std::cout << "Members still on the channel: " << channel->getMember().size() << std::endl;

	std::string msg = ":" + client->getNick() + "@" + client->getHostname() + " PART " + channel->getChannelName();
	sendMsg(msg, client->getFd());
	if (channel->getMember().size() > 0)
		sendMsgToAllMembers(msg, client->getFd());
	else if (channel->getMember().size() == 0)
		removeChannel(channel);
}

void	Server::KICK(Client *client, Channel *channel) {
	std::cout << "cmd Kick" << std::endl;

	//PARSING CHAN
	std::string chan = parseChan(command, 0);
	if (channel->isOperator(client) == true) {
		//CHECK IF THE CHAN EXIST
		if (!channelExists(chan))
			sendErrorMsg(ERR_NOSUCHCHANNEL, client->getFd(), channel->getChannelName(), "", "", "");
		//PARSING REASON
		size_t doublePoints = command.find(':');
		std::string reason = command.substr(doublePoints + 1);
		//PARSING NICK
		size_t startPos = command.find(chan) + chan.size() + 1; 
		size_t endPos = command.find(" :");
		std::string nick = command.substr(startPos, endPos - startPos);
		if (!channel->isMember(client)) {
			sendErrorMsg(ERR_NOTONCHANNEL, client->getFd(), channel->getChannelName(), "", "", "");
			return;
		}
		if (!channel->isNickMembre(nick)) {
			sendErrorMsg(ERR_USERNOTINCHANNEL, client->getFd(), client->getNick(), channel->getChannelName(), "", "");
			return;
		}
		//SEND MSG
		std::string msg = ':' + client->getNick() + "!~" + client->getHostname() + ' ' + token + ' ' + chan + ' ' + nick + " :" + reason;
		if (chan.empty() || nick.empty())
			sendErrorMsg(ERR_NEEDMOREPARAMS, client->getFd(), chan, nick, "", "");
		
		else {
			sendMsg(msg, client->getFd());
			sendMsgToAllMembers(msg, client->getFd());
		}
	}
	else {
		// std::string msg = channel->getChannelName() + " You must be a channel operator";
		sendErrorMsg(ERR_CHANOPRIVSNEEDED, client->getFd(), client->getNick(), chan, "Not allowed", "");
	}
}

void	Server::INVITE(Client *client, Channel *channel) {
		std::cout << "cmd invite" << std::endl;

		
	size_t chanPos = command.find("#");
	if (chanPos != std::string::npos) {
		std::string invited = command.substr(0, chanPos - 1);
		
		size_t spacePos = command.find(" ", chanPos + 1);
		std::string chanName;
		if (spacePos != std::string::npos) {
			chanName = command.substr(chanPos, spacePos - (chanPos + 1));
		} else {
			chanName = command.substr(chanPos);
		}		 
		
		if (channelExists(chanName) == 0)
		{
			std::cout << "channel null" << std::endl;
			sendErrorMsg(ERR_NOSUCHCHANNEL, client->getFd(), chanName, "", "", "");
			return;
		}

		else if (channel->isMember(client) == false)
				sendErrorMsg(ERR_NOTONCHANNEL, client->getFd(), chanName, "", "", "");

		else if (channel->isOperator(client) == false)
				sendErrorMsg(ERR_CHANOPRIVSNEEDED, client->getFd(), chanName, "", "", "");
		
		else if (isNickUsed(invited) == false)
				sendErrorMsg(ERR_NOSUCHNICK, client->getFd(), invited, "", "", "");

		else if (channel->isMember(invited) == true)
				sendErrorMsg(ERR_USERONCHANNEL, client->getFd(), chanName, invited, "", "");
		else
		{
			
			for(std::vector<Client*>::iterator it = _clients.begin(); it != _clients.end(); it++)
			{
				if (invited == (*it)->getNick())
				{
					channel->addGuest(*it);
					std::string	rply = "341 " + client->getNick() + " " + invited + " " + chanName + END_SEQUENCE;
					sendMsg(rply, client->getFd()); // ca permet d'avoir le msg 

					std::string msg = ":" + client->getNick() + " " + token + " " + invited + " " + chanName;
					sendMsg(msg, client->getFd()); // la ce me fait le message dans le chan

					std::string msg2 = ":" + client->getNick() + " " + token + " " + invited + " " + chanName;
					sendMsg(msg2, (*it)->getFd()); // ca ca envoie le msg a momo

					break ;
				}	
			}
		}	
	}
}

void	Server::PASS(Client *client, Channel *channel) {
	(void)channel;
	std::string pass = command;
	if (pass.empty()){
		std::cout << "PASS" << std::endl;
		sendErrorMsg(ERR_NEEDMOREPARAMS, client->getFd(), client->getNick(), "", "", "");
		return ;
	}
	if (pass != getPassword()){
		std::cout << "PASS" << std::endl;
		sendErrorMsg(ERR_PASSWDMISMATCH, client->getFd(),"", "", "", "");
		// exit(EXIT_FAILURE);
		return ;
	}
	else {
		if (client->isAuthenticated() == true){
			sendErrorMsg(ERR_ALREADYREGISTERED, client->getFd(),"", "", "", "");
			return ;
		}
		else {
			client->setIsAuthenticated(true);
			// if (std::find(_tokens.begin(), _tokens.end(), "PASS") == _tokens.end()) {
			// 	// "PASS" n'est pas dans _tokens
			// 	_tokens.push_back("PASS");
			// }
		}
	}
}

void Server::QUIT(Client *client, Channel *channel) {
	(void)channel;
	std::cout << "Client " << client->getNick() << " has quit." << std::endl;

	// Envoyer un message de départ aux autres clients si nécessaire mais on peut l'enlever car pas demande apparement
	// std::string quitMessage = "Client " + client->getNick() + " has quit.";
	// for (size_t i = 0; i < _clients.size(); i++) {
	// 	if (_clients[i].getFd() != client->getFd()) {
	// 		sendMsg(quitMessage, _clients[i].getFd());
	// 	}
	// }

	// Fermer la connexion du client
	int clientSocket = client->getFd();
	close(clientSocket);

	for (std::vector<Channel*>::iterator it = _channels.begin(); it != _channels.end(); it++)
	{
		if ((*it)->isGuest(client))
			((*it)->removeGuest(client));

		if ((*it)->isOperator(client))
			((*it)->removeOperator(client));

		if ((*it)->isMember(client))
			(*it)->removeMember((client), client->getFd());
	}

	// delete (client);
	// _clients.erase(client);

	// Supprimer l'objet Client du vecteur _clients
	for (size_t i = 0; i < _clients.size(); i++) {
		if (_clients[i]->getFd() == clientSocket) {
			delete (client);
			_clients.erase(_clients.begin() + i);
			break;
		}
	}
}