#ifndef BYTECODE_HPP
#define BYTECODE_HPP 1

#include <stdint.h>
#include <string>
#include <iostream>
#include <map>

namespace bytecode {
	typedef unsigned char byte;


	enum Instruction {
		NOP = 0,

		// stack ops
		PUSH = 1,     // X, pushes an integer value onto the stack
		PSHB,         // X, pushes a boolean value onto the stack
		//DROP,         // pops a value and discards it
		//SWAP,         // exchanges two top values
		//DUP,          // duplicates top value.

		ISP = 10,     // X, adds X to the stack pointer
		DSP,          // X, subtracts X from the stack pointer.

		// mem ops
		FETCH_X = 22,  // X, an address, pushes the value of X.
		STORE_X,       // X, pops a value, which is stored @X.

		// control flow
		JMP = 40,     // X, jumps to argument (line num)
		JMPTRUE,      // X, jump if true. pops a value. If it's != 0, then jumps to argument
		JMPFALSE,     // X, jump if false. pops a value. if it's = 0, then jumps to argument
		CALL,         // pushes the current line number, then pushes the current routine number. Then yields control to the procedure specified by the followup string.
		RET,          // pops a value, which is a routine number, then another, which is a line number in that routine. control yielded.
		INTERRUPT,     // X, call for built-in procedures and functions. no followup.

		// comparisons
		//CMP = 50,   // pops two values, subtracts the second from the first, and pushes the result.
		LESS = 51,    // pops two values, pushes true if the first is less than the second, false otherwise
		GREATER,      // pops two values, pushes true if the first is greater than the second, false otherwise
		EQ,           // pops two values, pushes true if they're equal, false otherwise
		NEQ,          // pops two values, pushes true if they're not equal, false otherwise

		// int ops - but these will be used for other types as well if the Great Big Idea is implemented.
		ADDI = 60,
		SUBI,
		MULI,
		DIVI,
		MODI,

		// boolean ops
		AND = 80,
		OR,
		NOT,

		// meta
		DELAY = 200,   // X, the interpreter releases the puppet for a delay of X ticks.

		// anotations
		PROC = 220,     // followed by proc name in c-string.
	}; // MUST NOT exceed 255

	bool has_argument(Instruction i);
	bool has_followup(Instruction i);


	class subprogram {
		public:
			static std::string normalize_name(std::string str);

		private:
			std::string name;

		public:
			byte const* code;
			size_t len;

		void set_name(const std::string& str);

		public:
			subprogram(std::string name, byte* code, size_t len);

			const std::string& get_name() const;

			// fetches the int at program_counter and increments program_counter by four.
			byte get_byte(size_t& program_counter) const;

			// fetches the byte at program_counter and increments program_counter by one.
			int get_int(size_t& program_counter) const;

			// fetches the C string at program_counter, and increments progrma_counter by len(string);
			// this would be better given back as a proper cstring
			std::string get_string(size_t& program_counter) const;


			// prints as pure bytecode
			void print_bytecode(std::ostream& out);

			// prints 'assembly'
			void print_assembly(std::ostream& out);

	};


	class builtin_call {
		private:
			static const std::map<std::string, builtin_call> mapping;
			static const std::map<std::string, builtin_call> init_mapping();

		public:
			const unsigned int identifier;
			const byte args;
			builtin_call(unsigned int identifier, byte args) : identifier(identifier), args(args) {}

			static bool call_exists(const std::string& name);
			static builtin_call get_call(const std::string& name);
	};


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
}

#endif