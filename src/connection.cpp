#include "connection.hpp"
#include "util/debug.hpp"
#include <sstream>
#include <string>

sum::Connection::Listener::Listener(Connection* socket) : running(true), socket(socket) {}

void sum::Connection::Listener::Run() {
	sf::Packet packet;
	std::string msg;
	sf::SelectorTCP selector;
	selector.Add(*socket);

	while(running) {
		if(selector.Wait(0.05f) > 0) {
			socket->Receive(packet);
			packet >> msg;
			debugf("Message from server: %s\n",msg.c_str());
		}
	}
}

void sum::Connection::Listener::PleaseDoStop() {
	this->running = false;
}

sum::Connection::Connection(std::vector<Observer<ServerMessage>*> &_observers) : 
	connected(false), 
	ip("255.255.255.255"), 
	port(0), 
	listener(0),
	observers(_observers)  {
	SetBlocking(true);
}

bool sum::Connection::connect(std::string address, unsigned short port) {
	disconnect();
	debugf("Trying to connect to server at %s:%d\n", address.c_str(), port);
	sf::Socket::Status status = Connect(port, sf::IPAddress(address), 4.0f); //timeout of whopping 4 seconds
	if( status != sf::Socket::Done ) {
		debugf("Could not connect to %s:%d\n", address.c_str(), port);
		return false;
	} else debugf("Connected. Sending scripts...\n");

	this->ip = address;
	this->port = port;

	//sending scripts
	sf::Packet packet;
	packet << 25;
	Send(packet);

	//getting reply.
	Receive(packet);
	std::string repl;
	packet >> repl;
	debugf("Handshake finished: %s.\n", repl.c_str());

	listener = new Listener(this);

	//adding observers
	for(int i=0 ; i<observers.size() ; ++i)
		listener->addObs(observers[i]);

	listener->Launch();

	return connected=true;
}

void sum::Connection::disconnect() {
	if(!connected) return;
	debugf("Closing connection to server %s\n", get_address().c_str());
	Close();
	listener->PleaseDoStop();
	delete listener; //hm
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

void sum::Connection::send(sf::Packet& packet) {
	Send(packet);
}
