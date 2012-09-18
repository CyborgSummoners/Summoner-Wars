#ifndef SEMANTICS_HPP
#define SEMANTICS_HPP 1

#include <vector>
#include <iostream>
#include <sstream>
#include "bytecode.hpp"

enum type{ boolean, integer };

struct var {
	unsigned int decl; // on which line was it declared?
	unsigned int writ; // on which line was it written into? 0 if never.
	unsigned int read; // on which line was it read? 0 if never.

	type typ;

	var(unsigned int decl=0, type typ = integer) : decl(decl), writ(0), read(0), typ(typ) {}
};

struct expression {
	type typ;
	codestream code;

	expression(type typ) : typ(typ) {}
};

struct statement {
	codestream code;
};



#endif
