void	Server::INVITE(Client *client, Channel *channel) {
		std::cout << "cmd invite" << std::endl;

		
	size_t chanPos = command.find("#");
	if (chanPos != std::string::npos) {
		std::string invited = command.substr(0, chanPos - 1);
		
		// Trouver la position du premier espace après le nom du canal (#chan)
		size_t spacePos = command.find(" ", chanPos + 1);
		std::string chanName;
		if (spacePos != std::string::npos) {
			// Si un espace est trouvé, extraire le nom du canal (#chan)
			chanName = command.substr(chanPos, spacePos - (chanPos + 1));
		} else {
			// Sinon, le reste de la commande est le nom du canal (#chan)
			chanName = command.substr(chanPos);
		}

		std::cout << "invited [" << invited << "]" << std::endl;
		std::cout << "channame [" << chanName << "]" << std::endl;
		std::cout << "channelExists " << channelExists(chanName) << std::endl;
		 
		//std::vector<std::string>	params;
		//comment mettre command dans mon vector params ?
		/*
		1. parser la commande : parametre 1 = std::string invited = nickname de l'utilisateur invite dans le channel
								parametre 2 = #nom du channel
		
		2. if (parametre < 2)
				sendErrorMsg(ERR_NEEDMOREPARAMS, client->getFd(), invited, channelName, "", "");
		*/
		//if (channel == NULL || !channelExists(chanName))
		//if (channel == NULL || chanName != channel->getChannelName())
		if (channelExists(chanName) == 0)
		{
			std::cout << "channel null" << std::endl;
			sendErrorMsg(ERR_NOSUCHCHANNEL, client->getFd(), chanName, "", "", "");
			return;
		}

		else if (channel->isMember(client) == false)
				sendErrorMsg(ERR_NOTONCHANNEL, client->getFd(), chanName, "", "", "");

		else if (channel->isOperator(client) == false) //&& que le channel est en mode invitation seulement ??)
				sendErrorMsg(ERR_CHANOPRIVSNEEDED, client->getFd(), chanName, "", "", "");
		//Les serveurs PEUVENT rejeter la commande avec la valeur numérique ERR_CHANOPRIVSNEEDED. En particulier, ils DEVRAIENT la rejeter lorsque le canal est en mode "invitation seulement" et que l'utilisateur n'est pas un opérateur du canal.
		
		else if (isNickUsed(invited) == false)
				sendErrorMsg(ERR_NOSUCHNICK, client->getFd(), invited, "", "", "");

		else if (channel->isMember(invited) == true)
				sendErrorMsg(ERR_USERONCHANNEL, client->getFd(), chanName, invited, "", "");
		else
		{
			//std::string msg = "341 :" + client->getNick() + " INVITE " + invited + " " + chanName + END_SEQUENCE;
			std::string msg = ":" + client->getNick() + "!~" + client->getHostname() + " " + token + " " + invited + " " + chanName;
       		sendMsg(msg, client->getFd());
			sendMsgToAllMembers(msg, client->getFd());

			for(std::vector<Client*>::iterator it = _clients.begin(); it != _clients.end(); it++)
			{
				if (invited == (*it)->getNick())
				{
					channel->addMember(*it);
					break ;
				}
				// msg = ":" + invited + "@" + client->getHostname() + " JOIN " + (chanName);
				// sendMsg(msg, client->getFd());
				// currentChannel->getChannelName() = chanName;
				// msg = ":" + invited + "@" + client->getHostname() + " = " + (chanName) + " :" + currentChannel->getAllMembers();
				// sendMsg(msg, client->getFd());
				// sendMsgToAllMembers(msg, client->getFd());
						// send info to client
				msg = ":" + invited + "@" + client->getHostname() + " JOIN " + (chanName);
				sendMsg(msg, client->getFd());

				// // send info of all members in the channel
				msg = ":" + invited + "@" + client->getHostname() + " = " + (chanName) + " :" + currentChannel->getAllMembers();
				sendMsg(msg, client->getFd());				
			}
		}	
	}
}