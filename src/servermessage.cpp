#include "servermessage.hpp"
#include <sstream>

sum::ServerMessage& sum::ServerMessage::operator<<(const std::string& str) {
	if(!this->msg.empty()) this->msg.append(1, ServerMessage::SEP);
	this->msg.append(str);
	return *this;
}
sum::ServerMessage& sum::ServerMessage::operator<<(const char* str) {
	if(!this->msg.empty()) this->msg.append(1, ServerMessage::SEP);
	this->msg.append(str);
	return *this;
}

sum::ServerMessage& sum::ServerMessage::operator<<(int i) {	// convenience-ish
	std::stringstream ss;
	ss << i;
	if(!this->msg.empty()) this->msg.append(1, ServerMessage::SEP);
	this->msg.append( ss.str() );
	return *this;
}

std::vector<std::string> sum::ServerMessage::get_parsed_msg() const {
	return string_explode(msg, SEP);
}

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

const char sum::ServerMessage::SEP = '|';
