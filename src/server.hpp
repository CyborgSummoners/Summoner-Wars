#ifndef SERVER_HPP
#define SERVER_HPP 1

#include <SFML/Network.hpp>
#include <vector>


namespace sum {

class Server : public sf::Thread {
	struct client {
		sf::SocketTCP socket;
		sf::IPAddress ip;
	};

	private:
		sf::SocketTCP listener;
		sf::SelectorTCP selector;
		unsigned short port;

	public:
		Server(unsigned short port);
		virtual void Start();

	private:
		virtual void Run();
};

}

#endif
