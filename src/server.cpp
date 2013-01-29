#include "server.hpp"
#include "parser.hpp"
#include "util/debug.hpp"
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <cstdio>
#include <ctime>
#include <vector>
#include <deque>


sum::Server::Client::Client(const sf::SocketTCP socket, const sf::IPAddress ip) : client_id(nextid()), socket(socket), ip(ip), summonables(sum::Logic::default_templates) {}

const std::string sum::Server::Client::nextid() {
	std::stringstream ss;
	ss << maxid++;
	return ss.str();
}

bool sum::Server::Client::operator==(const Client& rhs) const {
	return this->socket == rhs.socket;
}
bool sum::Server::Client::operator==(const sf::SocketTCP& rhs) const {
	return this->socket == socket;
}

std::string sum::Server::Client::toString() const {
	return client_id.empty()? this->ip.ToString() : client_id;
}


sum::Server::Server(unsigned short port, tick sec_per_tick, step step_size) : state(Starting), port(port), sec_per_tick(sec_per_tick), step_size(step_size) {
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
	std::deque<ServerMessage>& outbox = world->advance(step_size);
	for(size_t i=0; i<outbox.size(); ++i) {
		Broadcast(outbox[i]);
	}
	outbox.clear();
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

	debugf("Server started listening on port %d\n", port);

	float elapsed = 0.0f;
	sf::Clock clock;
	while(running) {

		if(state == Playing) {
			elapsed = clock.GetElapsedTime();

			if(elapsed >= sec_per_tick) {
				clock.Reset();
				elapsed = 0.0f;

				Tick();

				elapsed = clock.GetElapsedTime();
				clock.Reset();
			}
		} else elapsed = 0.0f;

		sockets = selector.Wait(sec_per_tick - elapsed);
		for(size_t i=0; i<sockets; ++i) {
			socket = selector.GetSocketReady(i);

			if(socket == listener) { // new connection
				if(state == Joining) {	// only if we're in the joining phase (no reconnects yet).
					listener.Accept(client, &ip);
					debugf("Got connection from %s, awaiting scripts\n", ip.ToString().c_str());
					waiting_list.push_back( new Client(client, ip) );
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
					Client* client = find_client(socket);

					if(state != Joining && client == 0) {
						fprintf(stderr, "Unknown client tried to send something but we're already playing. This is likely an error. I wish we used proper logging.\n");
						selector.Remove(socket);
						socket.Close();
					}
					else if(client == 0) { // connected, but no scripts yet, so this should be the push.
						// is she really on the waiting list?
						for(std::list<Client*>::iterator lit = waiting_list.begin(); lit != waiting_list.end(); ++lit) {
							if( *(*lit) == socket ) {
								client = *lit;
								break;
							}
						}
						if(client == 0) {
							fprintf(stderr, "Message from unknown client, closing connection. This is an error.\n");
							selector.Remove(socket);
							socket.Close();
						}
						else {
							using namespace sum::Parser;	//vajon miért nem tudja kitalálni?

							// now handshake
							try {
								waiting_list.remove(client);
								clients.push_back(client);

								packet.Clear();
								packet << "ack";
								socket.Send(packet);

								// send list of registered subprograms?

								// send available server functions to client:
								for(server_fun_iter fit = server_functions.begin(); fit != server_functions.end(); ++fit) {
									Send(
										*client,
										ServerMessage(ServerMessage::server_fun) << fit->first
									);
								}
								// send available puppet types to client:
								for(Logic::pup_template_map::const_iterator fit = client->summonables.begin(); fit != client->summonables.end(); ++fit) {
									Send(
										*client,
										ServerMessage(ServerMessage::register_mons) << fit->first << fit->second.toString()
									);
								}

								Broadcast(
									ServerMessage(ServerMessage::unknown) << "join" << ip.ToString() << client->client_id,
									*client
								);
							} catch(std::exception& e) {
								debugf("Got malformed packet instead of scripts from client @%s, closing connection.\n", client->ip.ToString().c_str());
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
						debugf("%s request %s\n", client->toString().c_str(), msg_handle.c_str());

						server_fun_iter callee = server_functions.find(msg_handle);
						if(callee == server_functions.end()) {
							debugf("No such handle: %s\n", msg_handle.c_str());
							Send(
								*client,
								ServerMessage(ServerMessage::reply) << "Fatal: no function called " << msg_handle << " found"
							);
						}
						else {
							const std::string repl = (this->*(callee->second))(*client, packet);
							Send( *client,ServerMessage(ServerMessage::reply, repl) );
						}
					}
				}
				else {	// close or error
					Client* client = find_client(socket);
					clients.remove(client);
					debugf("Client %s disconnected.\n", client->ip.ToString().c_str());
					Broadcast(
						ServerMessage(ServerMessage::connections) << "leave" << client->toString(),
						*client
					);
					socket.Close();
					selector.Remove(socket);

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
	for(std::list<Client*>::iterator lit = clients.begin(); lit != clients.end(); ++lit) {
		if(except == *(*lit))  continue;

		(*lit)->socket.Send(packet);
	}
}

sum::Server::Client* sum::Server::find_client(sf::SocketTCP socket) {
	for(std::list<Client*>::iterator lit = clients.begin(); lit != clients.end(); ++lit) {
		if( (*lit)->socket == socket) return *lit;
	}
	return 0;
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

	world = new Logic::World(interpreter, 50,50);

	// generic data
	sm << stringutils::float_to_string(sec_per_tick) // a tick is this many seconds
	   << step_size       // this many steps are in a tick.
	   << 50              // map x
	   << 50              // map y
	   << clients.size()  // játékosok száma
	;
	// create summoners;
	size_t num = 0;
	for(std::list<Client*>::iterator lit = clients.begin(); lit != clients.end(); ++lit) {
		Logic::Summoner& s = world->create_summoner(
			Logic::default_startpos(Logic::coord(50,50), clients.size(), num++),	//default starting pos
			(*lit)->client_id
		);
		sm << (*lit)->client_id // client's id
		   << s.get_id()     // summoner's actor id
		   << s.get_pos().x  // pos_x
		   << s.get_pos().y  // pos_y
		;

		debugf("Created summoner for %s...\n", (*lit)->toString().c_str());
	}


	state = Playing;
	Broadcast(sm);
}


//************************
//*** server functions ***
//************************
const std::string sum::Server::shout(Client& client, sf::Packet& packet) {
	std::string args;
	packet >> args;
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

const std::string sum::Server::serverdate(Client& client, sf::Packet& packet) {
	time_t raw;
	struct tm* tms;
	char buf[80];

	time(&raw);
	tms = localtime(&raw);

	strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S %Z", tms);

	return std::string(buf);
}

const std::string sum::Server::summon(Client& client, sf::Packet& packet) {
	if(state != Playing) return "Fatal: you can only summon things while playing.";
	std::string args;
	packet >> args;

	std::string Result = "";
	std::vector<std::string> parts = string_explode(args, stringutils::whitespace);
	// expecting format "summon <summonable> [<coord_x> <coord_y>]";
	std::string actor_type;
	unsigned int x;
	unsigned int y;
	bool success = true;
	size_t bit = 0;

	// gather args
	for(size_t i=0; i<parts.size(); ++i) {
		if(parts[i].empty()) continue;
		if(bit == 0) {
			actor_type = parts[i];
			//does it exsist?
			if(client.summonables.find(actor_type) == client.summonables.end()) {
				Result = "Error: first argument does not name a valid summonable (check your /mon directory for creatures you can summon).\n";
				success=false;
			}

			++bit;
		}
		else if(bit == 1) {
			if(!stringutils::to_unsigned(parts[i], x)) {
				Result = "Error: second argument should be the x coordinate.\n";
				success=false;
			}
			++bit;
		}
		else if(bit == 2) {
			if(!stringutils::to_unsigned(parts[i], y)) {
				Result = "Error: third argument should be the y coordinate.\n";
				success=false;
			}
			++bit;
		}
	}

	// enough args?
	if(success) {
		if(bit < 1) {
			Result = "Error: too few arguments to function.\n";
			success=false;
		}
		else if(bit > 3) {
			Result = "Error: too many arguments to function.\n";
			success=false;
		}
		else if(bit > 1 && bit < 3) {
			Result = "Error: too few arguments to function.\n";
			success=false;
		}
	}

	if(success) {
		if(bit == 1) {
			x = y = 0; // Fixme: set x,y to target. once target is implemented, that is.
		}

		Logic::Puppet* p = world->create_puppet(
			Logic::coord(x, y),
			client.client_id,
			client.summonables.find(actor_type)->second,
			Result
		);

		if(!p) {
			debugf("Summon failed: %s\n", Result.c_str());
			return "Error: " + Result;
		}

		debugf("%s summoned %s to (%d,%d)\n", client.toString().c_str(), actor_type.c_str(), x, y);
		return "";
	}
	return Result.append("Usage: summon <summonable> [<x-coord> <y-coord>]");
}

const std::string sum::Server::puppetinfo(Client& client, sf::Packet& packet) {
	if(state < Playing) return "Fatal: can't describe things when not playing.";
	std::string args;
	packet >> args;
	std::string Result = "";

	// args expected to contain puppet-id.
	std::vector<std::string> parts = string_explode(stringutils::trim(args), stringutils::whitespace);
	size_t puppet_id;
	if(!stringutils::to_unsigned(parts[0], puppet_id)) {
		Result = "Error: argument not an actor-id.";
	}
	else {
		return world->describe(puppet_id);
	}

	return Result.append("Usage: describe <actor-id>");
}


const std::string sum::Server::register_script(Client& client, sf::Packet& packet) {
	using sum::Parser::operator>>;

	sf::Uint32 len;
	bytecode::subprogram prog;
	packet >> len;
	debugf("Recieving %d scripts from client #%s.\n", len, client.toString().c_str());

	std::stringstream Result;
	size_t s=0;
	try {
		for(size_t i=0; i<len; ++i) {
			packet >> prog;	// this may throw
			prog.owner = client.client_id;

			if( interpreter.register_subprogram(prog) ) ++s;
			else {
				Result << "Could not register " << prog.get_name() << ": subprogram already exists.\n";
			}
		}
	} catch(std::exception& e) {
		debugf("Malformed packet from client #%s.\n", client.toString().c_str());
		return "Got malformed packet. Some scripts may not have been registered.\n";
	}

	debugf("Got %d / %d scripts from client #%s.\n", len, s, client.toString().c_str());

	return Result.str();
}


const std::map<std::string, sum::Server::server_function> sum::Server::initialize_server_functions() {
	std::map<std::string, server_function> Result;
	Result.insert( make_pair("shout", &Server::shout) );
	Result.insert( make_pair("serverdate", &Server::serverdate) );
	Result.insert( make_pair("summon", &Server::summon) );
	Result.insert( make_pair("describe", &Server::puppetinfo) );
	Result.insert( make_pair("scriptreg", &Server::register_script) );
	return Result;
}

const sum::Server::Client sum::Server::nobody = sum::Server::Client(sf::SocketTCP(), sf::IPAddress());
int sum::Server::Client::maxid = 0;
const std::map<std::string, sum::Server::server_function> sum::Server::server_functions = sum::Server::initialize_server_functions();
