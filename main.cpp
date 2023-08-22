// #include "Socket.hpp" // NADIA
// #include "Server.hpp" // NADIA
#include "./inc/Server.hpp"//Verena

// mode d'emploi : ./ircserv <port> <password>
int	main(int argc, char *argv[])
{
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// NADIA
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	try
	{
		// [ ] Gestion d'erreur ici a ajouter lors du parsing

		// [X] SOCKET()
		// [X] SETSOCKOPT()
		// [X] BIND()
		// [X] LISTEN()

		// [ ] POLL() / SELECT()
		// [ ] ACCEPT()
		// [ ] SEND()
		// [ ] CLOSE()

		// struct sockaddr_in	addr;
		Server				srv;
		Client				cl;

		if (argc < 3)
			throw (Server::ServException(ERRMSG"bad argument. Please respect this format ./ircserv <port> <password>")); // usage ./ircserv <port> <password> a afficher

		srv.setPort(atoi(argv[1]));
		srv.createSocket();
		srv.setPassword(argv[2]);
		std::cout << "Port: " << srv.getPort() << "\nPassword: " << srv.getPassword() << std::endl; //@Verena (pour recuperer les infos de co)
		std::cout << "PASS" << std::endl;
		srv.allSockets(); // DEBUG ONLY
		// bzero(&srv._addr, sizeof(srv._addr));
		// while (true)
		// {
			// srv.pollConnection();
		srv.connection();
		// }
		return (0);
	}
	catch(const std::exception& e)
	{
		// std::cerr << errno << '\n';
		std::cerr << e.what() << '\n';
	}

	//*/
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////





	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// CLAIRE
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//* CODE DE CLAIRE ICI

	//*/
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////







///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// VERENA
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}