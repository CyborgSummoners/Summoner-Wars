#include "server.hpp"
#include "parser.hpp"
#include "util/debug.hpp"
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <cstdio>


bool sum::Server::Client::operator==(const Client& rhs) const {
	return this->socket == rhs.socket;
}

std::string sum::Server::Client::toString() const {
	return client_id.empty()? this->ip.ToString() : client_id;
}


sum::Server::Server(unsigned short port) : state(Starting), port(port), step_size(30) {
	if(!listener.Listen(port)) {
		std::stringstream s;
		s << "Server could not listen on port " << port << std::endl;
		throw std::invalid_argument(s.str().c_str());
	}

	selector.Add(listener);
	state = Setup;
}

void sum::Server::Start() {
	Launch();
}

bool sum::Server::Newgame(unsigned char num_of_players) {
	if(state != Setup) return false;
	this->num_of_players = num_of_players;
	state = Joining;
	return true;
}

void sum::Server::Tick() {
	bool result = interpreter.step(step_size);
	Broadcast(
		ServerMessage(ServerMessage::unknown, result? "something happened!" : "tick")
	);
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

		if(state == Playing) {
			elapsed = clock.GetElapsedTime();

			if(elapsed >= tick) {
				clock.Reset();
				elapsed = 0.0f;

				Tick();

				elapsed = clock.GetElapsedTime();
				clock.Reset();
			}
		} else elapsed = 0.0f;

		sockets = selector.Wait(tick - elapsed);

		for(size_t i=0; i<sockets; ++i) {
			socket = selector.GetSocketReady(i);

			if(socket == listener) { // new connection
				if(state == Joining) {	// only if we're in the joining phase (no reconnects yet).
					listener.Accept(client, &ip);
					debugf("Got connection from %s, awaiting scripts\n", ip.ToString().c_str());

					client_descr.socket = client;
					client_descr.ip = ip;
					waiting_list.push_back(client_descr);
					selector.Add(client);
				}
				else {
					listener.Accept(client, &ip);
					debugf("Got connection from %s, but we're not in the joining phase\n", ip.ToString().c_str());
					client.Close();
				}
			}
			else {
				if(socket.Receive(packet) == sf::Socket::Done) { // transmission ok
					client_descr = find_client(socket);

					if(state != Joining && client_descr == nobody) {
						fprintf(stderr, "Unknown client tried to send something but we're already playing. This is likely an error. I wish we used proper logging.\n");
						selector.Remove(socket);
						socket.Close();
					}
					else if(client_descr == nobody) { // connected, but no scripts yet, so this should be the push.
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
							using namespace sum::Parser;	//vajon miért nem tudja kitalálni?

							std::stringstream ss;
							ss << ++Client::maxid;
							client_descr.client_id = ss.str();

							// so, this must be the push, okay.
							// ellenőrizni kéne...
							sf::Uint32 len;
							std::string msg;
							try {
								packet >> len;
								bytecode::subprogram prog;
								for(size_t i=0; i<len; ++i) {
									packet >> prog;
									prog.owner = client_descr.client_id;
									interpreter.register_subprogram(prog);
								}
								debugf("Got %d scripts from client #%s.\n", len, client_descr.toString().c_str());

								waiting_list.remove(client_descr);
								clients.push_back(client_descr);

								packet.Clear();
								packet << "ack";
								socket.Send(packet);

								Broadcast(
									ServerMessage(ServerMessage::unknown) << "join" << ip.ToString() << client_descr.client_id,
									client_descr
								);

								if(clients.size() >= num_of_players) {
									debugf("%d players have gathered, we can now start playing.\n", clients.size());
									state = Playing;
									Broadcast(
										ServerMessage(ServerMessage::start) << "10" << "10" << "2"
									);
								}
							} catch(std::exception& e) {
								debugf("Got malformed packet instead of scripts from client @%s, closing connection.\n", client_descr.ip.ToString().c_str());
								packet.Clear();
								packet << "nack";
								socket.Send(packet);
								socket.Close();
							}
						}
					}
					else {
						packet >> msg_type;
						packet >> msg;
						debugf("%s says: \"%s\" (type %d)\n", client_descr.toString().c_str(), msg.c_str(), msg_type);

						if(msg_type == 0) {	//akkor ez egy shout. hát, izé.
							Broadcast(
								ServerMessage(ServerMessage::shout) << msg	//Todo: who shouts, where, etc
							);
						}
					}
				}
				else {	// close or error
					selector.Remove(socket);

					for(std::list<Client>::iterator lit = clients.begin(); lit != clients.end(); ++lit) {
						if(lit->socket == socket) {
							debugf("Client %s disconnected.\n", lit->ip.ToString().c_str());

							Broadcast(
								ServerMessage(ServerMessage::connections) << "leave" << lit->toString()
							);

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

void sum::Server::Broadcast(ServerMessage msg, const Client& except) {
	sf::Packet packet;
	packet << msg;
	Broadcast(packet, except);
}
void sum::Server::Send(Client& to, ServerMessage msg) {
	sf::Packet packet;
	packet << msg;
	to.socket.Send(packet);
}


const sum::Server::Client sum::Server::nobody;
int sum::Server::Client::maxid = 0;
