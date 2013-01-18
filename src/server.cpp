#include "server.hpp"
#include "util/debug.hpp"
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <cstdio>


bool sum::Server::Client::operator==(const Client& rhs) const {
	return this->socket == rhs.socket;
}

std::string sum::Server::Client::toString() const {
	return this->ip.ToString();
}


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
	bool running = true;

	sf::SocketTCP socket;
	sf::IPAddress ip;
	sf::SocketTCP client;
	sf::Packet packet;
	std::string msg;
	std::stringstream ss;
	Client client_descr;

	debugf("Server started listening on port %d\n", port);
	while(running) {
		size_t sockets = selector.Wait();
		for(size_t i=0; i<sockets; ++i) {
			socket = selector.GetSocketReady(i);

			if(socket == listener) { // new connection
				listener.Accept(client, &ip);
				debugf("Accepted connection from %s\n", ip.ToString().c_str());
					client_descr.socket = client;
					client_descr.ip = ip;
				clients.push_back(client_descr);
				selector.Add(client);

				packet.Clear();
				ss.str("");
				ss << "New player connected from " << ip.ToString() << ".";
				packet << ss.str();
				Broadcast(packet, client_descr);
			}
			else {
				if(socket.Receive(packet) == sf::Socket::Done) { // transmission ok
					packet >> msg;
					debugf("A client says: \"%s\"", msg.c_str());
				}
				else {	// close or error
					selector.Remove(socket);

					for(std::list<Client>::iterator lit = clients.begin(); lit != clients.end(); ++lit) {
						if(lit->socket == socket) {
							debugf("Client %s disconnected.\n", lit->ip.ToString().c_str());

							ss.str("");
							ss << "Client " << lit->toString() << " disconnected.";

							packet.Clear();
							packet << ss.str();
							Broadcast(packet, *lit);

							clients.erase(lit);
							break;
						}
					}

					// anybody left?
					if(clients.empty()) {
						debugf("No clients left, exiting.\n");
						running = false;
					}
				}
			}
		}
	}
}


void sum::Server::Broadcast(sf::Packet& packet, const Client& except = nobody) {
	for(std::list<Client>::iterator lit = clients.begin(); lit != clients.end(); ++lit) {
		if(except == *lit)  continue;

		lit->socket.Send(packet);
	}
}

const sum::Server::Client sum::Server::nobody;
