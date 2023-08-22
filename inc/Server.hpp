#ifndef SERVER_HPP
# define SERVER_HPP

# include <iostream>
# include <sys/socket.h>
# include <netinet/in.h> // sockaddr_in, IPPROTO_TCP
# include <arpa/inet.h> // hton*, ntoh*, inet_addr
# include <unistd.h>  // close
# include <cerrno> // errors
# include <stdexcept>
# include <vector>
# include <map>
# include <fcntl.h>
# include <sys/poll.h>
# include <string>
# include <cstring>
# include <sstream>
# include <algorithm>
# include <poll.h>
# include <cstdlib>
#include <sstream>

# include "Client.hpp"
# include "Channel.hpp"

# define RED "\e[31m"
# define RES "\e[0m"

# define ERROR -1						// Everywhere when -1 means error
# define BACKLOG 32						// Number of connections allowed on the incoming queue for listen()
# define TIMEOUT_NO_P -1				// Specifying a negative value in timeout means an infinite timeout (for poll())
# define TIMEOUT_NO_S NULL				// Specifying a negative value in timeout means an infinite timeout (for select())
# define TIMEOUT_YES (3 * 60 * 1000)	// 3 minutes
# define MAX_FD 200						// Number of maximum fds / may be replaced by a vector
# define ERRMSG RED"Error: " 			//parce que c est joli
# define END_SEQUENCE "\r\n"			//pour temriner les phrases
# define SERVNAME "The MoIRes Connection Server" //nom du serveur
# define CMDNBR 14						//tableau de commandes si besoin de le modifier en terme de nombre
# define CHANTYPES "&#"					//nickname definition

class Server
{
	public:
		Server();
		Server(Server const &src);
		Server	&operator=(Server const &rhs);
		~Server();

		Client		*currentClient;
		Channel		*currentChannel;

		std::string	token; //@Verena to have the token
		std::string	command; //@Verena ... to arrete de se faire chier
		bool		nickSet; // to set the nick once at the begining // a voir si devait etre enlever ou surtout pas (Nadia)
		bool		passIsValid; //pour forcer l utilisation d un mot de passe

		bool		createSocket();
		bool		connection();
		Client*		addClient(int fd);
		Channel* 	addChannel(std::string name);
		void		removeChannel(Channel *channel);
		bool		channelExists(std::string channel);
		bool		isNickUsed(std::string nick);
		Client* 	findClientByNick(const std::string& nick);
		void		addInvite(std::string &invited, Channel *channel);
		
		bool		canTheyJoinThisChannel(Client *client); // en construction dans JOIN
		void		inputClient(std::string buf, Client *client, Channel *channel);
		void		cmdSelection(std::string buf, Client *client);
		void		capOrNOt(Client *client);
		void		allSockets();					// useless at the moment
		// void	errorminator();					// TBD
		
		// void		addToken(std::string token);
		// std::vector<std::string>	getToken();

		int			getPort();
		int			getSocket();
		void		setPort(int port);
		std::string	getPassword();//@Verena to print the password entered
		void		setPassword(std::string pass);
		// Client		getClients(Client *client);
		Channel*	getCurrentChannel(std::string msgBuf);
		std::vector<std::string>	getCap();

		//COMMNANDS
		void		parseCommand(std::string buf);
		std::string	parseChan(std::string buf, size_t pos);
		std::vector<std::string>	parseModeCmd(std::string buf);
		std::string	modePassword(std::string buf);
		std::string	modeTargetMember(std::string buf);
		std::map<char, int>	setArgsByMode();
		void		sendMsg(std::string message, int fd);
		void		sendMsgToAllMembers(std::string message, int fd);
		void		getCapLs(std::string buf);
		void		first_message(Client *client);
		void		sendMsgServer(Client *Client); // pas encore ecrite correctement donc ca ne compilera pas avec
		// void		sendErrMsgServer(int errorCode, Client *client); // pas encore ecrite correctement donc ca ne compilera pas avec
		void 		sendErrorMsg(int errorCode, int fd, std::string param1, std::string param2, std::string param3, std::string info);
		//COMMANDS CALL
		void		commands(std::string cmd, Client *client, Channel *channel);

		//UTILS
		std::string	trim(const std::string& str);

		class ServException : public std::exception {
			public:
				ServException(const char* msg) : _msg(msg) {}
				const char* what() const throw() {
					return _msg;
				}

			private:
				const char* _msg;
		};

	private:
		int						_socket;
		int						_validity;
		int						_port;
		struct sockaddr_in		_addr;
		std::vector<int>		_sockets;	// maybe useless
		std::vector<pollfd>		_pfds;
		std::vector<Client*>	_clients;	// vecteur clients pour faire le lien entre les classes ? @Verena
		std::vector<Channel*>	_channels;
		// std::vector<std::string>	_tokens; // pour la verification du mot de passe 
		// bool					_quit;		// useless at the moment
		std::string				_password; //@Verena to get the password
		int						_lastPing;
		std::string 			_cmdArray[CMDNBR];

		void					CAP(Client *client, Channel *channel);
		void					PING(Client *client, Channel *channel);
		void					NICK(Client *client, Channel *channel);
		void					USER(Client *client, Channel *channel);
		void					JOIN(Client *client, Channel *channel);
		void					MODE(Client *client, Channel *channel);
		void					PRIVMSG(Client *client, Channel *channel);
		void					NOTICE(Client *client, Channel *channel);
		// void					TOPIC(Client *client);
		void					TOPIC(Client *client, Channel *channel);
		void					PART(Client *client, Channel *channel);
		void					KICK(Client *client, Channel *channel);
		void					INVITE(Client *client, Channel *channel);
		void					PASS(Client *client, Channel *channel);
		void					QUIT(Client *client, Channel *channel);
};

#endif