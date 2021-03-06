#ifndef BYTECODE_HPP
#define BYTECODE_HPP 1

#include <stdint.h>
#include <string>
#include <iostream>
#include <map>
#include <stdexcept>

namespace bytecode {
	struct underflow : public std::runtime_error {
		underflow() : runtime_error("Bytecode underflow!") {}
	};


	enum type{ meta, none, any, boolean, integer, list, string, puppet, self };
	bool subtypeof(type super, type sub);

	typedef unsigned char byte;
	typedef char signed_byte;

	enum Instruction {
//		name          // arglen followup description
		NOP = 0,      //                 no operation.
                      //
		// stack ops  //
		PUSH = 1,     // 4               pushes an integer value onto the stack
		PSHB,         // 1               pushes a boolean value onto the stack
		PSHS,         //        CSTR     pushes a string literal onto the stack, given by the followup, a nullterminated string.
		PUSH_SELF,    //                 pushes a reference to Self, the puppet who runs the program
		COPY,         //                 pops a value from the stack, copies it, and pushes the copy.
		DCOPY,        //                 pops a value from the stack, performs deep copy on it, and pushes the copy.
//		DROP,         //                 pops a value and discards it
//		SWAP,         //                 exchanges two top values
//		DUP,          //                 duplicates top value.

		// mem ops
		RSRV = 20,    // 1               reserve space for arg local variables.
		FETCH_X = 22, // 1               an address, pushes the value of local variable or parameter @arg
		STORE_X,      // 1               pops a value, and stores it @arg (local variable or parameter)

		// control flow
		JMP = 40,     // 4               jumps to arg (byte number in bytecode)
		JMPTRUE,      // 4               jump if true. pops a value. If it's != false, then jumps to arg
		JMPFALSE,     // 4               jump if false. pops a value. if it's = true, then jumps to arg
		CALL,         //        CSN      passes control to the procedure specified by the followup string.
		RET,          //                 RETurn. Rewinds the stack to before all agruments. Pushes no value.
		RETV,         //                 RETurn Value. Pops a value from the stack, rewinds the stack to before all arguments, then pushes the value.
		INTERRUPT,    // 4               passes control to interrupt #arg.
		APPLY,        //                 Pops a string value, then CALLs the function specified by that value.

		// comparisons. These MUST be continuous, their order MUST NOT change.
		EQ = 50,      //                 pops two values, pushes true if they're equal, false otherwise
		NEQ,          //                 pops two values, pushes true if they're not equal, false otherwise
		LESS,         //                 pops two values, pushes true if the first is less than the second, false otherwise
		GREATER,      //                 pops two values, pushes true if the first is greater than the second, false otherwise

		// ops. These MUST be continuous, their order MUST NOT change.
		ADDI = 60,    //
		SUBI,         //
		MULI,         //
		DIVI,         //
		MODI,         //
		AND,          //
		OR,           //
		NEG,          //

		// list ops.
		LIST = 80,    // 4               pops arg values, builds a list out of them, and pushes the list.
		FETCH_IDX,    //                 pops a value V1, then another V2, then pushes the value of V1[V2]
		STORE_IDX,    //                 pops a value V1, then two others V2 and V3, and stores V3 @V1[V2].

		// meta
		DELAY = 200,   // 4              the interpreter releases the puppet for a delay of X ticks.

		// anotations
		PROC = 220,     //      CSN      in a bytecode file signifies start of procedure named followup.
	}; // MUST NOT exceed 255

	bool has_argument(Instruction i);
	size_t arglen(Instruction i);
	bool has_followup(Instruction i);
	int get_interrupt_id(const std::string& str);

	class subprogram {
		public:
			static std::string normalize_name(std::string str);

		private:
			std::string name;
			byte argc;
			bool retval;

			byte* code;
			size_t len;

			void set_name(const std::string& str);

		public:
			std::string owner; //namespace, if you really wish. kicsit hack, valójában külön deskriptor kéne a parsernek és a szervernek, de ez van.

		public:
			// konstruktorok
			subprogram();
			subprogram(std::string name, byte argc, byte* code, size_t len, bool retval = false);
			subprogram(const subprogram& prog);
			subprogram& operator=(const subprogram& prog);
			virtual ~subprogram();

			const std::string& get_name() const;
			byte get_argc() const;
			size_t get_codelen() const;
			const byte* access_code() const;

			// fetches the byte at program_counter, and increments program_counter by one.
			byte get_byte(size_t& program_counter) const;

			// fetches the int at program_counter, and increments program_counter by four.
			int get_int(size_t& program_counter) const;

			// fetches the C string at program_counter, and increments progrma_counter by len(string);
			// this would be better given back as a proper cstring
			std::string get_string(size_t& program_counter) const;

			void get_bytecode(byte*& Result, size_t& length);

			// prints as pure bytecode
			void print_bytecode(std::ostream& out);

			// prints 'assembly'
			void print_assembly(std::ostream& out);

		friend class Parser;
	};
}

#endif
