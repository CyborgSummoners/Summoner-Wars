#ifndef SERVER_HPP
#define SERVER_HPP 1

#include <SFML/Network.hpp>
#include <set>
#include <vector>
#include <string>
#include <map>
#include "bytecode.hpp"
#include "servermessage.hpp"
#include "serverlogic.hpp"
#include "measurements.hpp"
#include "interpreter.hpp"


namespace sum {

class Server : public sf::Thread {
	enum GameState { Starting, Setup, Joining, Playing, Done };

	struct Client {
		private:
			static int maxid;
			static const std::string nextid();

		public:
			const std::string client_id;
			sf::SocketTCP socket;
			const sf::IPAddress ip;

			Logic::pup_template_map summonables;

			Client(const sf::SocketTCP socket, const sf::IPAddress ip);
			void set_id();

			bool operator==(const Client& rhs) const;
			bool operator==(const sf::SocketTCP& rhs) const;
			std::string toString() const;
	};

	static const Client nobody;

	private:
		sf::SocketTCP listener;
		sf::SelectorTCP selector;
		GameState state;
		unsigned char num_of_players;
		unsigned short port;

		std::list<Client*> waiting_list;
		std::list<Client*> clients;

		tick sec_per_tick;	// a tick is this many seconds.
		step step_size;

		Interpreter interpreter;
		Logic::World* world;

	public:
		Server(unsigned short port, tick sec_per_tick = 1.0f, step step_size = 30);
		~Server();

		void Start();
		bool Newgame(unsigned char num_of_players);

	private:
		void Tick();
		void Run();
		void Broadcast(sf::Packet& packet, const Client& except = nobody);
		void Broadcast(ServerMessage msg, const Client& except = nobody);
		void Send(Client& to, ServerMessage msg);

		Client* find_client(sf::SocketTCP socket);

		void gamestart();

		// server functions the client can call
		typedef const std::string (sum::Server::*server_function)(Client&, sf::Packet&);
		static const std::map<std::string, server_function> server_functions;
		static const std::map<std::string, server_function> initialize_server_functions();
		typedef std::map<std::string, server_function>::const_iterator server_fun_iter;

		const std::string shout(Client& client, sf::Packet& packet);
		const std::string serverdate(Client& client, sf::Packet& packet);
		const std::string summon(Client& client, sf::Packet& packet);
		const std::string puppetinfo(Client& client, sf::Packet& packet);
		const std::string register_script(Client& client, sf::Packet& packet);
};

}

#endif
