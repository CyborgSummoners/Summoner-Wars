#ifndef CONNECTION_HPP
#define CONNECTION_HPP 1

#include <SFML/Network.hpp>
#include <string>

namespace sum {

class Connection : private sf::SocketTCP {
	private:
		bool connected;
		sf::IPAddress ip;
		unsigned short port;

	public:
		Connection();

	public:
		bool connect(std::string address, unsigned short port);
		void disconnect();
		bool is_connected() const;
		std::string get_address() const;

		//void send(sf::Packet& packet);
};

}

#endif
