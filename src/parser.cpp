#include "parser.hpp"
#include "compiler/summparse.h"
#include "util/debug.hpp"
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace {
	bytecode::byte* get_bytes_from_packet(sf::Packet& packet, size_t size) {	// bah
		bytecode::byte* Result = new bytecode::byte[size];
		size_t i = 0;

		while(i<size && !packet.EndOfPacket()) packet >> Result[i++];
		while(i<size) Result[i++] = 0;

		return Result;
	}
}

sf::Packet& sum::Parser::operator<<(sf::Packet& packet, const bytecode::subprogram& program) {
	packet << program.get_name();
	packet << program.get_argc();
	packet << static_cast<sf::Uint32>(program.get_codelen());
	packet.Append(program.access_code(), program.get_codelen());

	return packet;
}

sf::Packet& sum::Parser::operator>>(sf::Packet& packet, bytecode::subprogram& program) {
	std::string name;
	bytecode::byte argc;
	sf::Uint32 codelen;
	bytecode::byte* code = 0;

	packet >> name;
	packet >> argc;
	packet >> codelen;
	code = get_bytes_from_packet(packet, codelen);

	program = subprogram(name, argc, code, codelen);

	return packet;
}


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

	Result << static_cast<sf::Uint32>(parser.subprograms.size());

	for(size_t i=0; i<parser.subprograms.size(); ++i) {
		Result << parser.subprograms[i];
	}

	return Result;
}


