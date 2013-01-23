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

		coord(size_t x, size_t y);
		bool operator<(const coord& rhs) const;
	};

	coord default_startpos(coord map_size, size_t player_num, size_t which);

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

			Summoner& create_summoner(coord pos);
			Puppet& create_puppet(coord pos);

			coord get_pos(Actor& actor);
	};


	class Actor {
		static size_t maxid;
		public:
			static size_t gen_id();

		protected:
			const size_t id;
			World& my_world;
			size_t hp;
			Facing facing;

		public:
			virtual tick move() { return 10; };
			virtual tick turn_left() { return 10; };
			virtual tick turn_right() { return 10; };

			size_t get_id();
			coord get_pos();

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
