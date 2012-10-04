#ifndef COMPILER_HPP
#define COMPILER_HPP 1

#include "../bytecode.hpp"
#include <iostream>
#include <sstream>
#include <vector>
#include <deque>
#include <string>
#include <cctype>

using namespace bytecode;

struct codeline {
	uint32_t label;
	Instruction opcode;
	uint32_t argument;
	std::string followup;
	int line_no;

	codeline(Instruction opcode, uint32_t argument, uint32_t label=0, std::string followup="")
		: label(label), opcode(opcode), argument(argument), followup(followup), line_no(-1) {}

	void print() {
		if(line_no > -1) {
			std::cout << line_no << " -\t";
		}
		else std::cout << "\t";
		if(label != 0) std::cout << label << ":";
		std::cout << "\t" << opcode << "\t" << argument;
		if(followup.length() > 0) std::cout << " '" << followup << "'";
		std::cout << std::endl;
	}
};

typedef std::deque<codeline> codelines;
typedef std::deque<codeline>::iterator code_iterator;

struct var {
	unsigned int decl; // on which line was it declared?
	unsigned int writ; // on which line was it written into? 0 if never.
	unsigned int read; // on which line was it read? 0 if never.
	int num; // signed -- this is important. Parameters have negative numbers.

	type typ;

	var(unsigned int num, unsigned int decl=0, type typ = integer) : decl(decl), writ(0), read(0), num(num), typ(typ) {}

	bool is(type typ) const;
};

struct expression {
	type typ;
	codelines code;

	expression(type typ) : typ(typ) {}

	bool is(type typ) const;
};

struct statement {
	codelines code;
};

#endif
