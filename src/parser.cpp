#include "parser.hpp"
#include "compiler/summparse.h"
#include "util/debug.hpp"
#include <fstream>
#include <iostream>
#include <stdexcept>

sf::Packet sum::Parser::packetize_scripts_from_file(std::string fname, std::ostream& parser_output) {
	sf::Packet Result;
	std::ifstream source(fname.c_str());
	if(!source.is_open() || !source.good()) {
		debugf("Could not open source file '%s' for reading.", fname.c_str());
		throw std::invalid_argument("Could not open source file for reading");
		return Result;
	}

	::Parser parser(source, parser_output);
	parser.parse();

	Result << parser.subprograms.size();
	for(size_t i=0; i<parser.subprograms.size(); ++i) {
		Result << "muk";
	}

	return Result;
}
