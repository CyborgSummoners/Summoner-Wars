#include "server.hpp"
#include "parser.hpp"
#include "util/debug.hpp"
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <cstdio>
#include <ctime>


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

sum::Server::~Server() {
	delete world;
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
	std::string msg_handle;
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
							} catch(std::exception& e) {
								debugf("Got malformed packet instead of scripts from client @%s, closing connection.\n", client_descr.ip.ToString().c_str());
								packet.Clear();
								packet << "nack";
								socket.Send(packet);
								socket.Close();
							}

						if(clients.size() >= num_of_players) {
								gamestart();
							}
						}
					}
					else {
						packet >> msg_handle;
						packet >> msg;
						debugf("%s says: \"%s\" (handle %s)\n", client_descr.toString().c_str(), msg.c_str(), msg_handle.c_str());

						server_fun_iter callee = server_functions.find(msg_handle);
						if(callee == server_functions.end()) {
							debugf("No such handle: %s\n", msg_handle.c_str());
							Send(
								client_descr,
								ServerMessage(ServerMessage::reply) << "Fatal: no function called " << msg_handle << " found"
							);
						}
						else {
							const std::string repl = (this->*(callee->second))(client_descr, msg);	// hadd nyújtsam még át zárójelek e szerény csokrát: ((()))
							Send( client_descr,ServerMessage(ServerMessage::reply, repl) );
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

void sum::Server::gamestart() {
	debugf("%d players have gathered, we can now start playing.\n", clients.size());
	ServerMessage sm(ServerMessage::start);

	world = new Logic::World(50,50);

	// generic data
	sm << 50             // map x
	   << 50             // map y
	   << clients.size() // játékosok száma
	;
	// create summoners;
	size_t num = 0;
	for(std::list<Client>::iterator lit = clients.begin(); lit != clients.end(); ++lit) {
		Logic::Summoner& s = world->create_summoner(Logic::default_startpos(Logic::coord(50,50), clients.size(), num++));
		sm << lit->client_id // client's id
		   << s.get_id()     // summoner's actor id
		   << s.get_pos().x  // pos_x
		   << s.get_pos().y  // pos_y
		;

		debugf("Created summoner for %s...\n", lit->toString().c_str());
	}


	state = Playing;
	Broadcast(sm);
}


//************************
//*** server functions ***
//************************
const std::string sum::Server::shout(Client& client, std::string args) {
	debugf("SHOUT from %s: %s\n", client.toString().c_str(), args.c_str());

	// todo: trim args
	if(args.empty()) {
		return "Usage: shout <thing to shout>";
	}

	Broadcast(
		ServerMessage(ServerMessage::shout) << client.client_id << args	//Todo: who shouts, where, etc
	);

	return "";
}

const std::string sum::Server::serverdate(Client& client, std::string args) {
	time_t raw;
	struct tm* tms;
	char buf[80];

	time(&raw);
	tms = localtime(&raw);

	strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S %Z", tms);

	return std::string(buf);
}

const std::map<std::string, sum::Server::server_function> sum::Server::initialize_server_functions() {
	std::map<std::string, server_function> Result;
	Result.insert( make_pair("shout", &Server::shout) );
	Result.insert( make_pair("date", &Server::serverdate) );
	return Result;
}


const sum::Server::Client sum::Server::nobody;
int sum::Server::Client::maxid = 0;
const std::map<std::string, sum::Server::server_function> sum::Server::server_functions = sum::Server::initialize_server_functions();
