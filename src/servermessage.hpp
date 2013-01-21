#ifndef SERVERMESSAGE_HPP
#define SERVERMESSAGE_HPP

#include "include.hpp"
#include <SFML/Network.hpp>

namespace sum
{

struct ServerMessage
{

	enum Type{
		// meta:
		unknown    = 0,
		connections,
		disconnect,

		// map related:
		move       = 10,
		attack,
		spell,
		death,
		summon,

		// chatting
		shout      = 50,
		chat,

		// global state change
		win        = 100,
		start
	};

	ServerMessage(Type _type=unknown,std::string _msg="") :
		type(_type), msg(_msg){}

	Type type;
	std::string msg;

};

sf::Packet& operator<<(sf::Packet& packet, const ServerMessage& message);
sf::Packet& operator>>(sf::Packet& packet, ServerMessage& message);

}

#endif
