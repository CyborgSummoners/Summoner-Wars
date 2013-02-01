#include "combatlog.hpp"

namespace sum
{

void CombatLog::update(const ServerMessage &message)
{
	using namespace stringutils;

	std::vector<std::string> res = message.get_parsed_msg();

	switch(message.type)
	{
		case ServerMessage::hp_loss:
			add("Puppet with ID:" + res[0] + " lost " + res[1] + " hp.");
		break;
		default:
		break;
	}

}

}
