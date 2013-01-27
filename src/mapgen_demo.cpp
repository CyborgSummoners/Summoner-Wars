#include <SFML/System.hpp>
#include <iostream>
#include <sstream>
#include <cstring>
#include "mapgen.hpp"

int main(int argc, char* argv[]) {
	using namespace sum;
	using namespace sum::Mapgen;

	Terrain* map = new Terrain[50*20];

	Map_generator* gen;

	if(argc > 1) {
		if(strcmp(argv[1], "caves") == 0) {
			int iter = 4;
			if(argc > 2) {
				std::stringstream ss;
				ss << argv[2];
				ss >> iter;
			}
			if(argc > 3) {
				int seed;
				std::stringstream ss;
				ss << argv[3];
				ss >> seed;
				sf::Randomizer::SetSeed(seed);
			}

			gen = new Caves(iter);
		}
		else if(strcmp(argv[1], "arena") == 0) {
			gen = new Arena;
		}
		else {
			std::cout << "Unknown map type. Valid types: arena, cave." << std::endl;
			return 0;
		}
	} else {
		gen = new Caves(3);
	}

	std::cout << "seed: " << sf::Randomizer::GetSeed() << std::endl;

	gen->generate(map, 50, 20);

	print_map(map, 50, 20, std::cout);

	delete gen;

	return 0;
}
