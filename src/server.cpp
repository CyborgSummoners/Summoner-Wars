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

void sum::Server::Tick() {
	bool result = interpreter.step(100);
	sf::Packet packet;
	packet << (result? "something happened!":"tick");
	Broadcast(packet);
}

void sum::Server::Run() {
	bool running = true;

	size_t sockets;
	sf::SocketTCP socket;
	sf::IPAddress ip;
	sf::SocketTCP client;
	sf::Packet packet;
	int msg_type;
	std::string msg;
	std::stringstream ss;
	Client client_descr;

	debugf("Server started listening on port %d\n", port);

	float elapsed = 0.0f;
	sf::Clock clock;
	while(running) {
		elapsed = clock.GetElapsedTime();

		if(elapsed >= tick) {
			clock.Reset();
			elapsed = 0.0f;

			Tick();

			elapsed = clock.GetElapsedTime();
			clock.Reset();
		}

		sockets = selector.Wait(tick - elapsed);

		for(size_t i=0; i<sockets; ++i) {
			socket = selector.GetSocketReady(i);

			if(socket == listener) { // new connection
				listener.Accept(client, &ip);
				debugf("Got connection from %s, awaiting scripts\n", ip.ToString().c_str());

				client_descr.socket = client;
				client_descr.ip = ip;
				waiting_list.push_back(client_descr);
				selector.Add(client);
			}
			else {
				if(socket.Receive(packet) == sf::Socket::Done) { // transmission ok
					client_descr = find_client(socket);

					if(client_descr == nobody) { // connected, but no scripts yet, so this should be the push.
						// is she really on the waiting list?
						for(std::list<Client>::iterator lit = waiting_list.begin(); lit != waiting_list.end(); ++lit) {
							if(lit->socket == socket) {
								client_descr = *lit;
								break;
							}
						}
						if(client_descr == nobody) {
							fprintf(stderr, "Message from unknown client, closing connection. This is an error.\n");
							selector.Remove(socket);
							socket.Close();
						}
						else {
							// so, this must be the push, okay.
							int len;
							packet >> len;	//this many subprograms, I guess
							debugf("Got %d scripts from client @ %s.\n", len, client_descr.toString().c_str());

							packet.Clear();
							packet << "ack";
							socket.Send(packet);

							waiting_list.remove(client_descr);
							clients.push_back(client_descr);

							packet.Clear();
							ss.str("");
							ss << "New player connected from " << ip.ToString() << ".";
							packet << ss.str();
							Broadcast(packet, client_descr);
						}
					}
					else {
						packet >> msg_type;
						packet >> msg;
						debugf("%s says: \"%s\" (type %d)\n", client_descr.toString().c_str(), msg.c_str(), msg_type);

						if(msg_type == 0) {	//akkor ez egy shout. hát, izé.
							packet.Clear();
							ss.str("");
							ss << client_descr.toString() << " shouts: \"" << msg << "\"";
							packet << ss.str();
							Broadcast(packet);
						}
					}
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
					socket.Close();

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


void sum::Server::Broadcast(sf::Packet& packet, const Client& except) {
	for(std::list<Client>::iterator lit = clients.begin(); lit != clients.end(); ++lit) {
		if(except == *lit)  continue;

		lit->socket.Send(packet);
	}
}

sum::Server::Client sum::Server::find_client(sf::SocketTCP socket) {
	for(std::list<Client>::iterator lit = clients.begin(); lit != clients.end(); ++lit) {
		if(lit->socket == socket) {
			return *lit;
		}
	}
	return nobody;
}

const sum::Server::Client sum::Server::nobody;
