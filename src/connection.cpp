#include "connection.hpp"
#include "util/debug.hpp"
#include <sstream>
#include <iostream>

sum::Connection::Connection() : connected(false), ip("255.255.255.255"), port(0)  {
	SetBlocking(true);
}


bool sum::Connection::connect(std::string address, unsigned short port) {
	if(connected) {
		debugf("Closing connection to server %s", get_address().c_str());
		Close();
	}
	debugf("Trying to connect to server at %s:%d\n", address.c_str(), port);
	sf::Socket::Status status = Connect(port, sf::IPAddress(address), 4.0f);
	if( status != sf::Socket::Done ) {	//timeout of whopping 4 seconds
		debugf("Could not connect to %s:%d\n", address.c_str(), port);
		return false;
	} else debugf("Connected.\n");

	this->ip = address;
	this->port = port;

	return connected=true;
}

void sum::Connection::disconnect() {
	if(!connected) return;
	Close();
	connected=false;
}

bool sum::Connection::is_connected() const {
	return connected;
}

std::string sum::Connection::get_address() const {
	std::stringstream ss;
	ss << ip.ToString() << ":" << port;
	return ss.str();
}
