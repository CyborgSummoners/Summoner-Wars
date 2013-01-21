#include "servermessage.hpp"

sf::Packet& sum::operator<<(sf::Packet& packet, const ServerMessage& message) {
	packet << static_cast<sf::Uint32>(message.type);
	packet << message.msg;
	return packet;
}

sf::Packet& sum::operator>>(sf::Packet& packet, ServerMessage& message) {
	sf::Uint32 t;
	packet >> t >> message.msg;
	message.type = static_cast<ServerMessage::Type>(t);
	return packet;
}

