#ifndef PARSER_HPP
#define PARSER_HPP 1

#include <string>
#include <ostream>
#include <vector>
#include <SFML/Network.hpp>
#include "bytecode.hpp"

namespace sum {
namespace Parser {

sf::Packet& operator<<(sf::Packet& packet, const bytecode::subprogram& program);
sf::Packet& operator>>(sf::Packet& packet, bytecode::subprogram& program);

sf::Packet packetize_scripts_from_file(std::string fname, std::ostream& parser_output = std::cout);

}
}

#endif
