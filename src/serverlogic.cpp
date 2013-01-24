#include "serverlogic.hpp"
#include <cassert>
#include <stdexcept>
#include <sstream>
#include "util/debug.hpp"

namespace sum {
namespace Logic {

coord::coord(size_t x, size_t y) : x(x), y(y) {}

bool coord::operator<(const coord& rhs) const {
	if(x < rhs.x) return true;
	return y < rhs.y;
}

coord default_startpos(coord map_size, size_t player_num, size_t which) {
	assert(player_num > 0 && player_num <= 4);
	assert(which < player_num);

	// tfh manhattan distance, tfh mapx ~= mapy
	if(player_num <= 2) {
		if(which == 0) return coord(0, map_size.y/2);
		if(which == 1) return coord(map_size.x - 1, map_size.y/2);
	}
	if(player_num == 3) { // kb egyenlő oldalú háromszög, FIXME: legyen középen
		if(which == 0) return coord(0, map_size.y/2);
		if(which == 1) return coord(map_size.y/2 , 0);
		if(which == 2) return coord(map_size.y/2 , map_size.y - 1);
	}
	if(player_num == 4) { // négy sarok, fixme
		if(which == 0) return coord(0,0);
		if(which == 1) return coord(0, map_size.y-1);
		if(which == 2) return coord(map_size.x-1, 0);
		if(which == 3) return coord(map_size.x-1, map_size.y-1);
	}

	return coord(0,0);
}


World::World(size_t width, size_t height) : width(width), height(height) {}
World::~World() {
	for(std::map<coord, Actor*>::iterator it = puppets.begin(); it!=puppets.end(); ++it) {
		delete it->second;
	}
};

std::vector<ServerMessage> World::advance(step steps) {
	outbox.clear();
/*	outbox.push_back(
		ServerMessage(ServerMessage::unknown, "hey")
	);*/
	interpreter.advance(steps);
	return outbox;
}

Summoner& World::create_summoner(coord pos, const std::string& client_id, const std::vector<bytecode::subprogram>& progs, std::vector<bool>& reg_success) {
	Summoner* Result = new Summoner(*this);
	puppets.insert( std::make_pair(pos, Result) );	// FIXME check if it actually succeeded
	summoners.insert( std::make_pair(client_id, Result) );

	reg_success.resize(progs.size());
	for(size_t i=0; i<progs.size(); ++i) {
		reg_success[i] = interpreter.register_subprogram(progs[i]);
	}

	return *Result;
}

Puppet* World::create_puppet(coord pos, const std::string& client_id, const Puppet_template& attributes, std::string& failure_reason) {
	debugf("Creating new puppet for client %s... ", client_id.c_str());

	// do we know this summoner?
	std::map<std::string, Summoner*>::const_iterator sit = summoners.find(client_id);
	if( sit == summoners.end() ) {
		failure_reason = "Unknown summoner.";
		debugf("failed: %s\n", failure_reason.c_str());
		return 0;
	}
	Summoner* owner = sit->second;

	if(attributes.mana_cost > owner->mana) {
		failure_reason = "Not enough mana!";
		debugf("failed: %s\n", failure_reason.c_str());
		return 0;
	}

	// is anyone at pos?
	if( puppets.count( pos ) > 0 ) {
		failure_reason = "Position already occupied.";
		debugf("failed: %s\n", failure_reason.c_str());
		return 0;
	}

	Puppet* Result = new Puppet(*this, *owner, attributes);
	if( puppets.insert( std::make_pair(pos, Result) ).second == false) { //for weirdness...
		failure_reason = "Position already occupied.";
		debugf("failed: %s\n", failure_reason.c_str());
		delete Result;
		return 0;
	}

	interpreter.register_puppet(*Result);
	interpreter.set_behaviour(*Result, "DEMO", client_id);
	owner->mana -= attributes.mana_cost;

	debugf("Created puppet id %d.\n", Result->get_id());

	//post messages...
	return Result;
}

coord World::get_pos(Actor& actor) {
	for(std::map<coord, Actor*>::const_iterator it = puppets.begin(); it!=puppets.end(); ++it) {
		if(&actor == it->second) return it->first;
	}

	throw std::invalid_argument("No such puppet!");
}


size_t Actor::maxid = 0;
size_t Actor::gen_id() {
	return ++maxid;
}

Actor::Actor(World& my_world, attribute hp) : id(gen_id()), my_world(my_world), hp(hp), facing(north) {
}

size_t Actor::get_id() {
	return id;
}
coord Actor::get_pos() {
	return my_world.get_pos(*this);
}

std::string Puppet_template::toString() const {
	std::stringstream ss;
	ss << "mana_cost = " << mana_cost << "\n"
	   << "maxhp = " << maxhp << "\n"
	   << "move_cost = " << move_cost << "\n"
	   << "turn_left_cost = " << turn_left_cost << "\n"
	   << "turn_right_cost = " << turn_right_cost << "\n"
	;

	return ss.str();
}


Puppet::Puppet(World& my_world, const Summoner& owner, const Puppet_template& attributes) : Actor(my_world, 30), owner(owner), attributes(attributes) {
	this->hp = attributes.maxhp;
}

step Puppet::move() {
	debugf("%d wants to move.\n", this->get_id());
	return attributes.move_cost;
}
step Puppet::turn_left() {
	debugf("%d wants to turn left.\n", this->get_id());
	return attributes.turn_left_cost;
}
step Puppet::turn_right() {
	debugf("%d wants to turn right.\n", this->get_id());
	return attributes.turn_right_cost;
}
std::string Puppet::get_name() {
	std::stringstream ss;
	ss << this->get_id();
	return ss.str();
}
bool Puppet::operator==(const Puppet& that) {
	return this == &that;
}
bool Puppet::operator==(const Interpreter::Puppet& that) {
	return this == &that;
}


Summoner::Summoner(World& my_world) : Actor(my_world, 20), mana(100) {
}


const std::map<std::string, Puppet_template> init_default_templates() {
	std::map<std::string, Puppet_template> Result;
	//made up values, mostly.
	Puppet_template pup;
		pup.mana_cost = 10;
		pup.maxhp = 20;
		pup.move_cost = 40;
		pup.turn_left_cost = pup.turn_right_cost = 20;
	Result.insert( std::make_pair("robot", pup) );
		pup.mana_cost = 15;
		pup.maxhp = 40;
		pup.move_cost = 60;
		pup.turn_left_cost = pup.turn_right_cost = 30;
	Result.insert( std::make_pair("big_robot", pup) );

	return Result;
}

}
}
