#include "serverlogic.hpp"
#include <cassert>
#include <stdexcept>
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


World::World(size_t width, size_t height) : width(width), height(height) {
}

Summoner& World::create_summoner(coord pos) {
	Summoner* Result = new Summoner(*this);
	puppets.insert( std::make_pair(pos, Result) );	// FIXME check if it actually succeeded
	return *Result;
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

Actor::Actor(World& my_world, size_t hp) : id(gen_id()), my_world(my_world), hp(hp), facing(north) {
}

size_t Actor::get_id() {
	return id;
}
coord Actor::get_pos() {
	return my_world.get_pos(*this);
}


Puppet::Puppet(World& my_world, const Summoner& owner) : Actor(my_world, 30), owner(owner) {
}

Summoner::Summoner(World& my_world) : Actor(my_world, 20), magic(100) {
}

}
}
