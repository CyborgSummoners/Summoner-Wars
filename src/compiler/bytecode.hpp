#ifndef BYTECODE_HPP
#define BYTECODE_HPP 1

#include <stdint.h>
#include <vector>

enum Instruction {
	// stack ops
	PUSH = 1,     // X, pushes value onto the stack
	DROP,         // pops a value and discards it
	SWAP,         // exchanges two top values

	// mem ops
	FETCH = 20,  // pops a value, which is an address. pushes the value of that address.
	STORE,       // pops a value (address), then pops another. The second value is stored at the first.

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


//TODO, this should be a byte array proper (but endianness is a problem)
typedef std::vector<uint32_t> codestream;


void put_opcode(codestream& s, Instruction ins);

void put_dword(codestream& s, uint32_t ins);

void print_codestream(codestream& s);


#endif
