#ifndef SERVER_HPP
#define SERVER_HPP 1

#include <SFML/Network.hpp>
#include <list>
#include <string>


namespace sum {

class Server : public sf::Thread {
	struct Client {
		sf::SocketTCP socket;
		sf::IPAddress ip;

		bool operator==(const Client& rhs) const;
		std::string toString() const;
	};

	static const Client nobody;

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
		void Broadcast(sf::Packet& packet, const Client& except = nobody);

		Client find_client(sf::SocketTCP socket);
};

}

#endif
