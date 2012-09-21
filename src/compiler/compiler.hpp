#ifndef COMPILER_HPP
#define COMPILER_HPP 1

#include "../bytecode.hpp"
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <cctype>

using namespace bytecode;

enum type{ boolean, integer };

struct var {
	unsigned int decl; // on which line was it declared?
	unsigned int writ; // on which line was it written into? 0 if never.
	unsigned int read; // on which line was it read? 0 if never.
	unsigned int num;

	type typ;

	var(unsigned int decl=0, type typ = integer) : decl(decl), writ(0), read(0), typ(typ) {
		numgen();
	}

	void numgen() {
		static int last = 0;
		num = last++;
	}
};

struct expression {
	type typ;
	std::vector<codeline> code;

	expression(type typ) : typ(typ) {}
};

struct statement {
	std::vector<codeline> code;
};


#endif
