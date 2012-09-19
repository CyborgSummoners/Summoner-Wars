#ifndef BYTECODE_HPP
#define BYTECODE_HPP 1

#include <stdint.h>
#include <string>
#include <iostream>
#include <map>

namespace bytecode {

	enum Instruction {
		// stack ops
		PUSH = 1,     // X, pushes a (literal) value onto the stack
		DROP,         // pops a value and discards it
		SWAP,         // exchanges two top values
		DUP,          // duplicates top value.

		// mem ops
		FETCH = 20,  // pops a value, which is an address. pushes the value of that address.
		STORE,       // pops a value (address), then pops another. The second value is stored at the first.

		// comparisons
		CMP = 30,     // pops two values, subtracts the second from the first, and pushes the result.
		LESS,         // pops two values, pushes 1 if the first is less than the second, 0 otherwise
		GREATER,      // pops two values, pushes 1 if the first is greater than the second, 0 otherwise
		EQ,           // pops two values, pushes 1 if they're equal, 0 otherwise
		NEQ,          // pops two values, pushes 1 if they're not equal, 0 otherwise

		// control flow
		JMP = 40,     // X, jumps to argument (line num)
		JMPTRUE,      // X, jump if true. pops a value. If it's > 0, then jumps to argument
		JMPFALSE,     // X, jump if false. pops a value. if it's = 0, then jumps to argument
		CALL,         // X, pushes the current line number, then pushes the current routine number. Then yields control to X, which is a routine number.
		RET,          // pops a value, which is a routine number, then another, which is a line number in that routine. control yielded.

		// int ops
		ADDI = 60,
		SUBI,
		MULI,
		DIVI,
		MODI,

		// boolean ops
		AND = 80,
		OR,
		NOT
	}; // MUST NOT exceed 256

	struct codeline {
		std::string label;
		Instruction opcode;
		uint32_t argument;

		codeline(std::string label, Instruction opcode, uint32_t argument) : label(label), opcode(opcode), argument(argument) {}

		void print() {
			std::cout << opcode << " " << argument << std::endl;
		}
	};
}

#endif
