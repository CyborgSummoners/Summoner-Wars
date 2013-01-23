#ifndef SERVERLOGIC_HPP
#define SERVERLOGIC_HPP 1

#include <cstdlib>
#include <map>
#include <string>

namespace sum {
namespace Logic {
	typedef size_t step;
	typedef int attribute;

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
	struct Puppet_template;

	class World {
		size_t width;
		size_t height;
		std::map<coord, Actor*> puppets;

		public:
			World(size_t width, size_t height);

			Summoner& create_summoner(coord pos);
			Puppet& create_puppet(coord pos, Summoner& owner);

			coord get_pos(Actor& actor);
	};


	class Actor {
		static size_t maxid;
		public:
			static size_t gen_id();

		protected:
			const size_t id;
			World& my_world;
			attribute hp;
			Facing facing;

		public:
			virtual step move() { return 10; };
			virtual step turn_left() { return 10; };
			virtual step turn_right() { return 10; };

			size_t get_id();
			coord get_pos();

		protected:
			Actor(World& my_world, attribute hp);

		friend class World;
	};


	struct Puppet_template {
		attribute mana_cost;
		attribute maxhp;
		// los range, firing range, shooting power, melee attributes...
		step move_cost;
		step turn_left_cost;
		step turn_right_cost;
		// map of default overrides, eventually

		std::string toString() const;
	};


	class Puppet : public Actor {
		const Summoner& owner;
		Puppet_template attributes; // buffs would modify these, I think.

		private:
			Puppet(World& my_world, const Summoner& owner, const Puppet_template& attributes);

		friend class World;
	};


	class Summoner : public Actor {
		attribute magic;

		private:
			Summoner(World& my_world);

		friend class World;
	};

	typedef std::map<std::string, Puppet_template> pup_template_map;
	const pup_template_map init_default_templates();
	const pup_template_map default_templates = init_default_templates();
}
}

#endif
