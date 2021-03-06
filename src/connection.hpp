#ifndef CONNECTION_HPP
#define CONNECTION_HPP 1

#include <SFML/Network.hpp>
#include "observable.hpp"
#include "servermessage.hpp"
#include <string>

namespace sum {

class Connection : private sf::SocketTCP {
	class Listener : public sf::Thread, public Observable<ServerMessage> {
		bool running;
		Connection* socket;
		public:
			Listener(Connection* socket);
			void PleaseDoStop();
		private:
			void Run();
	};

	private:
		bool connected;
		sf::IPAddress ip;
		unsigned short port;
		Listener* listener;
		std::vector<Observer<ServerMessage>*> observers;

	public:
		Connection(std::vector<Observer<ServerMessage>*> &_observers);

	public:
		//call these in sequence.
		bool connect(const std::string& address, unsigned short port);
		bool handshake();
		void listen();

		void disconnect();
		bool is_connected() const;
		std::string get_address() const;

		void send(sf::Packet& packet);
};

}

#endif
