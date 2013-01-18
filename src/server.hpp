#ifndef SERVER_HPP
#define SERVER_HPP 1

#include <SFML/Network.hpp>
#include <list>


namespace sum {

class Server : public sf::Thread {
	struct Client {
		sf::SocketTCP socket;
		sf::IPAddress ip;

		bool operator==(const Client& rhs) const {
			return this->socket == rhs.socket;
		}
	};

	private:
		sf::SocketTCP listener;
		sf::SelectorTCP selector;
		unsigned short port;

		std::list<Client> clients;

	public:
		Server(unsigned short port);
		void Start();

	private:
		void Run();

};

}

#endif
