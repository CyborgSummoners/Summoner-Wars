#ifndef SEMANTICS_HPP
#define SEMANTICS_HPP 1

#include <vector>
#include <iostream>
#include <sstream>
#include "bytecode.hpp"
#include <vector>

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
		num = last;
		++last;
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
