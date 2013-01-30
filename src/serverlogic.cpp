#include "serverlogic.hpp"
#include <cassert>
#include <stdexcept>
#include <sstream>
#include "util/debug.hpp"

namespace sum {
namespace Logic {

std::string toString(Facing facing) {
	switch(facing) {
		case north:
			return "north";
		case west:
			return "west";
		case south:
			return "south";
		case east:
			return "east";
		default:
			return "TOWARDS A BUG";
	}
}

coord::coord(size_t x, size_t y) : x(x), y(y) {}

bool coord::operator<(const coord& rhs) const {
	if(x < rhs.x) return true;
	if(rhs.x < x) return false;
	return y < rhs.y;
}

coord coord::operator+(const coord& rhs) const {
	coord res(*this);
	res.x+=rhs.x;
	res.y+=rhs.y;
	return res;
}
coord coord::operator+(const Facing& facing) const {
	assert(facing < FACING_SIZE);
	coord res(*this);
	switch(facing) {
		case north:
			--res.y;
			break;
		case west:
			--res.x;
			break;
		case south:
			++res.y;
			break;
		case east:
			++res.x;
			break;
		default:
			break;
	}
	return res;
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


World::World(Interpreter& interpreter, size_t width, size_t height) : interpreter(interpreter), width(width), height(height) {}
World::~World() {
	for(std::map<coord, Actor*>::iterator it = puppets.begin(); it!=puppets.end(); ++it) {
		delete it->second;
	}
};

std::deque<ServerMessage>& World::advance(step steps) {
	// outbox.clear(); // clearing the outbox is the responsibility of the postman.
	interpreter.advance(steps);
	return outbox;
}

void World::post_message(const ServerMessage& msg) {
	outbox.push_back(msg);
}

step World::move_me(Puppet& actor) {
	debugf("%d moves ", actor.get_id());
	coord pos = get_pos(actor);
	coord respos = pos + actor.facing;
	step res = actor.attributes.move_cost;
	ServerMessage sm(ServerMessage::move);
	sm << actor.get_id()   // this actor tried to move
	   << pos.x            // from here
	   << pos.y
	   << respos.x         // to here
	   << respos.y
	;

	// valid?
	if(!is_valid(respos)) {
		debugf("and bumps into a wall.\n");
		sm << 0    // and fails;
		   << res  // it took him res many steps.
		;
		post_message(sm);
		return res;
	}

	// free?
	if(!is_free(respos)) {
		debugf("and bumps into another puppet.\n");
		sm << 0 << res;
		post_message(sm);
		return res;
	}

	debugf("to %d,%d.\n", respos.x, respos.y);

	puppets.erase( puppets.find(pos) );
	puppets.insert( std::make_pair(respos, &actor) );

	sm << 1   // success
	   << res // it took him res many steps.
	;

	post_message(sm);
	return res;
}

void World::hurt(Actor& actor, const size_t hp_loss) {
	actor.hp -= hp_loss;
	if(actor.hp <= 0) {
		actor.die();
	}
	else {
		post_message(
			ServerMessage(ServerMessage::hp_loss) << actor.get_id() // this puppet
			                                      << hp_loss // lost this much hp
			                                      << actor.hp // he has this much remaining.
		);
	}
}

void World::kill(Puppet& puppet) {
	debugf("puppet %d died\n", puppet.id);
	interpreter.unregister_puppet(puppet);
	for(std::map<coord, Actor*>::iterator it = puppets.begin(); it!=puppets.end(); ++it) {
		if(&puppet == it->second) {
			puppets.erase(it);
			break;
		}
	}

	post_message(
		ServerMessage(ServerMessage::death) << puppet.get_id() // this puppet died :(
											<< puppet.hp // because he had only this many hp.
	);

	delete &puppet;
}
void World::kill(Summoner& summoner) {
	debugf("A summoner died!\n");
}


Summoner& World::create_summoner(coord pos, const std::string& client_id) {
	Summoner* Result = new Summoner(*this);
	puppets.insert( std::make_pair(pos, Result) );	// FIXME check if it actually succeeded
	summoners.insert( std::make_pair(client_id, Result) );
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
	if( !is_free( pos ) > 0 ) {
		failure_reason = "Position already occupied.";
		debugf("failed: %s\n", failure_reason.c_str());
		return 0;
	}

	Puppet* Result = new Puppet(*this, *owner, attributes);
	if( puppets.insert( std::make_pair(pos, Result) ).second == false) { //should not be true
		assert(false);
		failure_reason = "Position already occupied. This should never happen.";
		debugf("failed: %s\n", failure_reason.c_str());
		delete Result;
		return 0;
	}

	interpreter.register_puppet(*Result);
	interpreter.set_behaviour(*Result, "DEMO", client_id);
	owner->mana -= attributes.mana_cost;

	debugf("Created puppet id %d.\n", Result->get_id());

	post_message(
		ServerMessage(ServerMessage::summon) << client_id            // this summoner-soul
		                                     << Result->get_id()     // summoned this creature
		                                     << Result->facing       // initial facing
		                                     << pos.x                // to these coordinates
		                                     << pos.y
		                                     << attributes.mana_cost // losing this much mana,
		                                     << owner->mana          // now she has thus much.
	);

	return Result;
}

coord World::get_pos(const Actor& actor) const {
	for(std::map<coord, Actor*>::const_iterator it = puppets.begin(); it!=puppets.end(); ++it) {
		if(&actor == it->second) return it->first;
	}

	throw std::invalid_argument("No such puppet!");
}

bool World::is_valid(coord pos) const {
	return pos.x>=0 && pos.y>=0 && pos.x<width && pos.y<height;
}

bool World::is_free(coord pos) const {
	return is_valid(pos) && puppets.count(pos)==0;
}

std::string World::describe(size_t actor_id) const {
	for(std::map<coord, Actor*>::const_iterator it = puppets.begin(); it!=puppets.end(); ++it) {
		if(it->second->get_id() == actor_id) return it->second->describe();
	}
	return "";
}
std::string World::describe(const Puppet& puppet) const {
	std::stringstream Result;
	coord pos = get_pos(puppet);
	Result  << "Postion: (" << pos.x << "," << pos.y << "), facing " << toString(puppet.facing) << std::endl
	        << "HP: " << puppet.hp << "/" << puppet.attributes.maxhp << std::endl
	        << "Behaviour: " << interpreter.get_behaviour(puppet) << std::endl
	;
	return Result.str();
}
std::string World::describe(const Summoner& summoner) const {
	std::stringstream Result;
	Result << "Summoner" << std::endl  // insert name, eventually
	       //<< "Facing " << toString(summoner.facing) << std::endl	//sounds silly
	       << "HP: " << summoner.hp << "/" << 20 << std::endl
	       << "Mana: " << summoner.mana << "/" << 100 << std::endl
	;
	return Result.str();
}
std::string Puppet::describe() const {
	return my_world.describe(*this);
}
std::string Summoner::describe() const {
	return my_world.describe(*this);
}


size_t Actor::maxid = 0;
size_t Actor::gen_id() {
	return ++maxid;
}

Actor::Actor(World& my_world, attribute hp) : id(gen_id()), my_world(my_world), hp(hp), facing(north) {
}

size_t Actor::get_id() const {
	return id;
}
coord Actor::get_pos() const {
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
	return my_world.move_me(*this);
}
step Puppet::turn_left() {
	Facing orig = this->facing;
	this->facing = static_cast<Facing>((this->facing + 1) % FACING_SIZE);
	debugf("%d turns left, facing %d.\n", this->get_id(), this->facing);
	my_world.post_message(
		ServerMessage(ServerMessage::turn) << this->id     // id of actor turning
		                                   << 0            // left or right? 0 or 1?
		                                   << orig         // from
		                                   << this->facing // to
		                                   << attributes.turn_right_cost //in this many steps
	);
	return attributes.turn_right_cost;
}
step Puppet::turn_right() {
	Facing orig = this->facing;
	this->facing = static_cast<Facing>((static_cast<unsigned>(this->facing) - 1) % FACING_SIZE);
	debugf("%d turns left, facing %d.\n", this->get_id(), this->facing);
	my_world.post_message(
		ServerMessage(ServerMessage::turn) << this->id     // id of actor turning
		                                   << 1            // left or right?
		                                   << orig         // from
		                                   << this->facing // to
		                                   << attributes.turn_right_cost //in this many steps
	);
	return attributes.turn_left_cost;
}
std::string Puppet::get_name() {
	std::stringstream ss;
	ss << this->get_id();
	return ss.str();
}
bool Puppet::is_alive() {
	return (hp > 0);
}
bool Puppet::operator==(const Puppet& that) {
	return this == &that;
}
bool Puppet::operator==(const Interpreter::Puppet& that) {
	return this == &that;
}

step Puppet::brain_damage(size_t severity, const std::string& message) {
	size_t hp_loss = severity * 5;
	step res = severity * 10;

	my_world.post_message(
		ServerMessage(ServerMessage::program_error) << this -> id  // id of puppet fouling himself
		                                            << message // description of problem
		                                            << res
	);

	my_world.hurt(*this, hp_loss);
	return res;
}

void Puppet::die() {
	my_world.kill(*this);
}
void Summoner::die() {
	my_world.kill(*this);
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
