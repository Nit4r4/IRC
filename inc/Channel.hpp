#ifndef CHANNEL_HPP
# define CHANNEL_HPP

# include "Server.hpp"
// # include "Client.hpp"

class Channel
{
	public:
			Channel();
			Channel(std::string name);
			Channel(Channel const &cpy);
			Channel &operator=(Channel const &rhs);
			~Channel();

			// GETTERS
			std::string					getChannelName();
			std::string					getTopic();
			std::string					getPassword();
			std::string					getAllMembers();
			std::vector<Client*>		getMember();
			std::vector<Client*>		getGuests();
			int							getNbLimit();
			bool						getTopicMode();
			bool						getLimitMode();
			bool						getPassMode();
			bool						getInviteMode();

			// SETTERS
			void						setTopic(std::string topic, Client *client);
			void						setTopicMode(bool mode);
			void						setPassMode(bool mode);
			void						setInviteMode(bool mode);
			void						setChannelPassword(std::string password);
			void						setLimit(bool mode, int limit);
			bool						setOperator(bool mode, std::string username);

			// METHODS
			void						addMember(Client *client);
			void						addGuest(Client *client);
			void						removeMember(Client *client, int fd);
			void						sendToAllMembers(std::string msg);
			bool						addOperator(Client *client);
			bool						removeOperator(Client *client);
			void						removeGuest(Client *client);
			bool						isOperator(Client *client);
			bool						isNumber(std::string arg);						
			bool						isMember(Client *client);
			bool 						isMember(const std::string& nickname);
			bool						isNickMembre(std::string nickname);
			bool						isGuest(Client *client);

	private:
			std::vector<Client*>		_members;
			std::vector<Client*>		_operators;
			std::vector<Client*>		_guests;

			std::string 				_channelName;
			std::string					_topic;
			std::string					_password;

			bool						_topicOperatorsOnly;
			bool						_isLimitSet;
			bool						_isPasswordSet;
			bool						_isInviteOnly;

			int							_nbLimit;
};

#endif
