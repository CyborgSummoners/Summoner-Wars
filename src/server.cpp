#include "server.hpp"
#include "util/debug.hpp"
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <cstdio>

sum::Server::Server(unsigned short port) : port(port) {
	if(!listener.Listen(port)) {
		std::stringstream s;
		s << "Server could not listen on port " << port << std::endl;
		throw std::invalid_argument(s.str().c_str());
	}

	selector.Add(listener);
}

void sum::Server::Start() {
	Launch();
}

void sum::Server::Run() {
	sf::SocketTCP socket;
	sf::IPAddress ip;
	sf::SocketTCP client;
	sf::Packet packet;
	std::string msg;

	debugf("Server started listening on port %d\n", port);
	while(true) {
		size_t sockets = selector.Wait();
		for(size_t i=0; i<sockets; ++i) {
			socket = selector.GetSocketReady(i);

			if(socket == listener) { // new connection
				listener.Accept(client, &ip);
				debugf("Accepted connection from %s\n", ip.ToString().c_str());
				selector.Add(client);
			}
			else {
				if(socket.Receive(packet) == sf::Socket::Done) { // transmission ok
					packet >> msg;
					debugf("Client says: \"%s\"", msg.c_str());
				}
				else {	// close or error
					debugf("Connection closed\n");
					selector.Remove(socket);
				}
			}
		}
	}
}

