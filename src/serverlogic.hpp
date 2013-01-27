#ifndef SERVERLOGIC_HPP
#define SERVERLOGIC_HPP 1

#include <cstdlib>
#include <map>
#include <string>
#include <deque>
#include "mapgen.hpp"
#include "interpreter.hpp"
#include "measurements.hpp"
#include "servermessage.hpp"

namespace sum {
namespace Logic {
	typedef int attribute;

	enum Facing {	// careful, we're doing math with it
		north,
		west,
		south,
		east,
		FACING_SIZE
	};

	std::string toString(Facing facing);

	struct coord {
		size_t x;
		size_t y;

		coord();
		coord(size_t x, size_t y);
		bool operator<(const coord& rhs) const;
		coord operator+(const coord& rhs) const;
		coord operator+(const Facing& facing) const;
	};

	coord default_startpos(coord map_size, size_t player_num, size_t which);

	class Actor;
	class Puppet;
	class Summoner;
	struct Puppet_template;

	class World {
		static const Map_generator& Default_mapgen;

		Interpreter interpreter;

		size_t width;
		size_t height;
		std::map<coord, Actor*> puppets;
		std::map<std::string, Summoner*> summoners;
		Terrain* terrain;

		std::deque<ServerMessage> outbox;

		public:
			World(size_t width, size_t height, const Map_generator* const mapgen = &World::Default_mapgen);
			~World();

			Summoner& create_summoner(coord pos, const std::string& client_id, const std::vector<bytecode::subprogram>& progs, std::vector<bool>& reg_success);
			Puppet* create_puppet(coord pos, const std::string& client_id, const Puppet_template& attributes, std::string& failure_reason);

			std::deque<ServerMessage>& advance(step steps);

			void post_message(const ServerMessage& msg);
			step move_me(Puppet& actor);

			coord get_pos(const Actor& actor) const;
			bool is_valid(coord pos) const;
			bool is_free(coord pos) const;

			std::string describe(size_t actor_id) const;

			Terrain terrain_at(coord pos) const;

			const Terrain* const get_map() const;
			std::string dump_mapdata() const;
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
			size_t get_id() const;
			coord get_pos() const;

			virtual std::string describe() const = 0;

		protected:
			Actor(World& my_world, attribute hp);

		public:
			virtual ~Actor() {}

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


	class Puppet : public Actor, public Interpreter::Puppet {
		const Summoner& owner;
		Puppet_template attributes; // buffs would modify these, I think.

		private:
			Puppet(World& my_world, const Summoner& owner, const Puppet_template& attributes);

			step move();
			step turn_left();
			step turn_right();
			std::string get_name();

			std::string describe() const;

			bool operator==(const Interpreter::Puppet& that);
			bool operator==(const Puppet& that);
		friend class World;
	};

	class Summoner : public Actor {
		attribute mana;

		private:
			Summoner(World& my_world);

			std::string describe() const;

		friend class World;
	};

	typedef std::map<std::string, Puppet_template> pup_template_map;
	const pup_template_map init_default_templates();
	const pup_template_map default_templates = init_default_templates();
}
}

#endif
