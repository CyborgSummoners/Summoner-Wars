#ifndef MAPGEN_HPP
#define MAPGEN_HPP 1

#include <SFML/System.hpp>
#include <cstdlib>
#include <ostream>

namespace sum {

	enum Terrain {
		floor = 1,
		wall = 2
	};

	struct Map_generator { // interface
		virtual void generate(Terrain*& map, size_t width, size_t height) = 0;
	};

	namespace Mapgen {
		void print_map(Terrain* map, size_t width, size_t height, std::ostream& out);

		struct Arena : public Map_generator {	// aka basic
			void generate(Terrain*& map, size_t width, size_t height);
		};

		struct Caves : public Map_generator {
			const unsigned short cycles;
			Caves(unsigned short cycles = 3) : cycles(cycles) {}

			void generate(Terrain*& map, size_t width, size_t height);
		};
	}
}

#endif
