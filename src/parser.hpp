#ifndef PARSER_HPP
#define PARSER_HPP 1

#include <string>
#include <iostream>
#include <SFML/Network.hpp>

namespace sum {
namespace Parser {

sf::Packet packetize_scripts_from_file(std::string fname, std::ostream& parser_output = std::cout);

}
}

#endif
