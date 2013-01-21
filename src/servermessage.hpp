#ifndef SERVERMESSAGE_HPP
#define SERVERMESSAGE_HPP

#include "include.hpp"

namespace sum
{

struct ServerMessage
{

	enum Type{
		// meta:
		unknown    = 0,
		connections,
		disconnect,
		reply,

		// map related:
		move       = 10,
		attack,
		spell,
		death,
		summon,

		// chatting
		shout      = 50,
		chat,

		// global
		win        = 100,
	};

	ServerMessage(Type _type=unknown,std::string _msg="") :
		type(_type), msg(_msg){}

	Type type;
	std::string msg;

};

}

#endif
