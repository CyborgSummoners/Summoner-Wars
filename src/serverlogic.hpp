#ifndef SERVERLOGIC_HPP
#define SERVERLOGIC_HPP 1

#include <cstdlib>
#include <map>

namespace sum {
namespace Logic {
	typedef size_t tick;

	struct coord {
		size_t x;
		size_t y;

		bool operator<(const coord& rhs);
	};

	enum Facing {
		north,
		west,
		south,
		east
	};

	class Actor;
	class Puppet;
	class Summoner;

	class World {
		size_t width;
		size_t height;
		std::map<coord, Actor*> puppets;

		public:
			World(size_t width, size_t height);

			Summoner& create_summoner(size_t x_pos, size_t y_pos);
			Puppet& create_puppet(size_t x_pos, size_t y_pos);
	};


	class Actor {
		protected:
			World& my_world;
			size_t hp;
			Facing facing;

		public:
			virtual tick move() = 0;
			virtual tick turn_left() = 0;
			virtual tick turn_right() = 0;

		protected:
			Actor(World& my_world, size_t hp);

		friend class World;
	};


	class Puppet : public Actor {
		const Summoner& owner;

		private:
			Puppet(World& my_world, const Summoner& owner);

		friend class World;
	};


	class Summoner : public Actor {
		size_t magic;

		private:
			Summoner(World& my_world);

		friend class World;
	};

}
}

#endif
