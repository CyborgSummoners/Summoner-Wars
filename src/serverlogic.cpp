#include "serverlogic.hpp"

namespace sum {
namespace Logic {

bool coord::operator<(const coord& rhs) {
	return x<rhs.x && y<rhs.y;
}


World::World(size_t width, size_t height) : width(width), height(height) {
}

Actor::Actor(World& my_world, size_t hp) : my_world(my_world), hp(hp), facing(north) {
}

Puppet::Puppet(World& my_world, const Summoner& owner) : Actor(my_world, 30), owner(owner) {
}

Summoner::Summoner(World& my_world) : Actor(my_world, 20), magic(100) {
}

}
}
