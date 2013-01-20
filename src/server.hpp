#ifndef SERVER_HPP
#define SERVER_HPP 1

#include <SFML/Network.hpp>
#include <list>
#include <string>
#include "interpreter.hpp"

namespace sum {

class Server : public sf::Thread {
	struct Client {
		static int maxid;

		std::string client_id;
		sf::SocketTCP socket;
		sf::IPAddress ip;

		bool operator==(const Client& rhs) const;
		std::string toString() const;
	};

	static const Client nobody;

	static const float tick = 1.0f;	// a tick is this many seconds.

	private:
		sf::SocketTCP listener;
		sf::SelectorTCP selector;
		unsigned short port;

		std::list<Client> waiting_list;
		std::list<Client> clients;

		Interpreter interpreter;

	public:
		Server(unsigned short port);
		void Start();

	private:
		void Tick();
		void Run();
		void Broadcast(sf::Packet& packet, const Client& except = nobody);

		Client find_client(sf::SocketTCP socket);
};

}

#endif
