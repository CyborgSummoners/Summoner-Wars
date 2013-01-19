#ifndef SERVERMESSAGE_HPP
#define SERVERMESSAGE_HPP

#include "include.hpp"

namespace sum
{

struct ServerMessage
{

	enum Type{
		unknown=0,
		move,
		attack,
		spell,
		death,
		summon,
		shout,
		chat,
		win
	};

	ServerMessage(Type _type=unknown,std::string _msg="fakkyeah") :
		type(_type), msg(_msg){}

	Type type;
	std::string msg;

};

}

#endif