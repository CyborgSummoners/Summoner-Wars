#ifndef SERVER_HPP
#define SERVER_HPP 1

#include <SFML/Network.hpp>
#include <list>
#include <string>
#include <map>
#include "interpreter.hpp"
#include "servermessage.hpp"
#include "serverlogic.hpp"

namespace sum {

class Server : public sf::Thread {
	enum GameState { Starting, Setup, Joining, Playing, Done };

	struct Client {
		static int maxid;

		std::string client_id;
		sf::SocketTCP socket;
		sf::IPAddress ip;

		Logic::Summoner* summoner;
		Logic::pup_template_map summonables;

		Client();

		bool operator==(const Client& rhs) const;
		std::string toString() const;
	};

	static const Client nobody;

	static const float tick = 1.0f;	// a tick is this many seconds.

	private:
		sf::SocketTCP listener;
		sf::SelectorTCP selector;
		GameState state;
		unsigned char num_of_players;
		unsigned short port;

		std::list<Client> waiting_list;
		std::list<Client> clients;

		Interpreter interpreter;

		size_t step_size;

		Logic::World* world;

	public:
		Server(unsigned short port);
		~Server();

		void Start();
		bool Newgame(unsigned char num_of_players);

	private:
		void Tick();
		void Run();
		void Broadcast(sf::Packet& packet, const Client& except = nobody);
		void Broadcast(ServerMessage msg, const Client& except = nobody);
		void Send(Client& to, ServerMessage msg);

		Client find_client(sf::SocketTCP socket);

		void gamestart();

		// server functions the client can call
		typedef const std::string (sum::Server::*server_function)(Client&, std::string);
		static const std::map<std::string, server_function> server_functions;
		static const std::map<std::string, server_function> initialize_server_functions();
		typedef std::map<std::string, server_function>::const_iterator server_fun_iter;

		const std::string shout(Client& client, std::string args);
		const std::string serverdate(Client& client, std::string args);
		const std::string summon(Client& client, std::string args);
};

}

#endif
