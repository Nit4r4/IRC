#include "../inc/Server.hpp"
#include "../inc/Client.hpp"
#include "../inc/Messages.hpp"

void Server::sendMsg(std::string message, int fd)
{
	std::cout << "SEND: " << message.append(END_SEQUENCE) << std::endl;
 	send(fd, message.c_str(), message.size(), 0);
}

void		Server::sendMsgToAllMembers(std::string message, int fd)
{
	std::vector<Client*>	members = currentChannel->getMember(); // moins moche apres
	// message = ":" + client->getNick() + "@" + client->getHostname() + " JOIN " + command;
	// message a envoyer a tous les membres du channel
	for (std::vector<Client*>::iterator it=members.begin(); it !=members.end(); it++)
	{
		if ((*it)->getFd() != fd) // || autorise a recevoir des messages
			sendMsg(message, (*it)->getFd());
	}
}


void    Server::first_message(Client *client) {

	std::string	msg = "001 " + client->getNick() + " :" + "\033[34mWelcome on the MoIRes Connection Server " + "!~" + client->getNick() + " @" + client->getHostname() + END_SEQUENCE + RES;
	sendMsg(msg, client->getFd());
}

void Server::sendErrorMsg(int errorCode, int fd, std::string param1, std::string param2, std::string param3, std::string info)
{
	std::stringstream ss;
	ss << errorCode;
	(void)info;
	std::string errorminator = ss.str();

	switch(errorCode)
	{
		case ERR_UNKNOWNERROR:
			errorminator += " UNKNOWNERROR " + param1 + param2 + " :Unknown error";
			break;
		case ERR_NOSUCHNICK:
			errorminator += " NOSUCHNICK " + param1 + " :No such nick/channel";
			break;
		case ERR_NOSUCHCHANNEL:
			errorminator += " NOSUCHCHANNEL " + param1 + " :No such channel";
			break;
		case ERR_CANNOTSENDTOCHAN:
			errorminator += " CANNOTSENDTOCHAN " + param1 + " :Cannot send to channel";
			break;
		case ERR_NORECIPIENT:
			errorminator += " NORECIPIENT :No recipient given (" + param1 + ")";
			break;
		case ERR_NOTEXTTOSEND:
			errorminator += " NOTEXTTOSEND :No text to send";
			break;
		case ERR_UNKNOWNCOMMAND:
			errorminator += " UNKNOWNCOMMAND " + param1 + " :Unknown command";
			break;
		case ERR_NONICKNAMEGIVEN:
			errorminator += " NONICKNAMEGIVE :No nickname given";
			break;
		case ERR_ERRONEUSNICKNAME:
			errorminator += " ERRONEUSNICKNAME " + param1 + " :Erroneous nickname";
			break;
		case ERR_NICKNAMEINUSE:
			errorminator += " NICKNAMEINUSE " + param1 + " :Nickname is already in use";
			break;
		case ERR_NICKCOLLISION:
			errorminator += " NICKCOLLISION " + param1 + " :Nickname collision KILL from " + param2 + "@" + param3;
			break;
		case ERR_USERNOTINCHANNEL:
			errorminator += " USERNOTINCHANNEL " + param1 + " " + param2 + " :They aren't on that channel";
			break;
		case ERR_NOTONCHANNEL:
			errorminator += " NOTONCHANNEL " + param1 + " :You're not on that channel";
			break;
		case ERR_USERONCHANNEL:
			errorminator += " USERONCHANNEL " + param1 + " " + param2 + " :is already on channel";
			break;
		case ERR_NEEDMOREPARAMS:
			errorminator += " NEEDMOREPARAMS " + param1 + param2 + " :Not enough parameters";
			break;
		case ERR_ALREADYREGISTERED:
			errorminator += " ALREADYREGISTRED :You may not reregister";
			break;
		case ERR_PASSWDMISMATCH:
			errorminator += " PASSWDMISMATCH :Password incorrect";
			break;
		case ERR_CHANNELISFULL:
			errorminator += " " + param1 + " " + param2 + " :Cannot join channel (+l)";
			break;
		case ERR_UNKNOWNMODE:
			errorminator += " " + param1 + " " + param2 + " :is not a recognised channel mode.";
			break;
		case ERR_INVITEONLYCHAN:
			errorminator += " ERR_INVITEONYLCHAN " + param1 + " " + param2 + " :Cannot join channel. (+i)";
			break;
		case ERR_BADCHANNELKEY:
			errorminator += " ERR_BADPASSWORD " + param2 + " :Cannot join channel (incorrect channel key)";
			break;
		case ERR_CHANOPRIVSNEEDED:
			errorminator += + " " + param1 + " " + param2 + " :You're not a channel operator";
			break;
		case ERR_UMODEUNKNOWNFLAG:
			errorminator += " UMODEUNKNOWNFLAG :Unknown MODE flag";
			break;
		case 650:
			errorminator += param1 + ":<channel> [:<topic>]";
			break;
		case ERR_INVALIDMODEPARAM:
			errorminator += + " " + param1 + " " + param2 + " " + param3 + " *" + " :You must specify a parameter for the " + param3 + " mode";
			break;
		default:
			errorminator += " " + param1 + " :Unknown error";
			break;
	}
	errorminator = ": " + errorminator + END_SEQUENCE ;
	sendMsg(errorminator, fd);
}
