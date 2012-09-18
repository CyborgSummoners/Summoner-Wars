#include "bytecode.hpp"
#include <iostream>

void put_opcode(codestream& s, Instruction ins) {
	s.push_back( static_cast<uint32_t>(ins) );
}

void put_dword(codestream& s, uint32_t ins) {
	s.push_back( ins );
}


void print_codestream(codestream& s) {
	for(size_t i=0; i<s.size(); ++i) {
		std::cout << s[i] << std::endl;
	}

}
