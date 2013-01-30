#include "bytecode.hpp"
#include "interpreter.hpp"
#include <iomanip>
#include <iostream>

namespace bytecode {

	bool subtypeof(type super, type sub) {
		return (
			   (super == sub)                     // everything qualifies as a subtype of itself.
			|| (super == any)                     // everything is a subtype to any.
			|| (super == list   && sub == string) // string is a list
			|| (super == puppet && sub == self)   // self is a puppet
		);
	}

	bool has_argument(Instruction i) {
		switch(i) {
			case PUSH:
			case PSHB:
			case RSRV:
			case FETCH_X:
			case STORE_X:
			case JMP:
			case JMPTRUE:
			case JMPFALSE:
			case LIST:
			case DELAY:
			case INTERRUPT:
				return true;
			default:
				return false;
		}
	}

	size_t arglen(Instruction i) {
		switch(i) {
			case PUSH:
			case JMP:
			case JMPTRUE:
			case JMPFALSE:
			case LIST:
			case INTERRUPT:
			case DELAY:
				return 4;
			case FETCH_X:
			case STORE_X:
			case PSHB:
			case RSRV:
				return 1;
			default:
				return 0;
		}
	}

	bool has_followup(Instruction i) {
		return (i==CALL || i==PSHS);
	}

	int get_interrupt_id(const std::string& str) {
		return sum::Interpreter::get_interrupt_id(str);
	}

 	std::string subprogram::normalize_name(std::string str) {
		if( str.size() > 15 ) str.resize(15);
		for(size_t i=0; i<str.length(); ++i) str[i] = toupper(str[i]);
		return str;
	}

	subprogram::subprogram() {
		set_name("NOP");
		argc = 0;
		code = new byte[6];
			code[0] = NOP;
			code[1] = DELAY;
			code[2] = code[3] = code[4] = 0;
			code[5] = 100;	// default behaviour: delay 100;
		len=6;
		retval = false;
	}

	subprogram::subprogram(const subprogram& prog) {
		set_name(prog.name);
		argc = prog.argc;
		len = prog.len;
		retval = prog.retval;
		owner = prog.owner;

		code = new byte[len];
		for(size_t i=0; i<len; ++i) {
			code[i] = prog.code[i];
		}
	}

	subprogram& subprogram::operator=(const subprogram& prog) {
		set_name(prog.name);
		argc = prog.argc;
		len = prog.len;
		retval = prog.retval;
		owner = prog.owner;

		if(this->code == prog.code) return *this; // selfassignment

		delete[] code;
		code = new byte[len];
		for(size_t i=0; i<len; ++i) {
			code[i] = prog.code[i];
		}

		return *this;
	}

	subprogram::subprogram(std::string name, byte argc, byte* code, size_t len, bool retval) : name(normalize_name(name)), argc(argc), retval(retval), code(code), len(len) {}

	subprogram::~subprogram() {
		delete[] code;
	}

	void subprogram::set_name(const std::string& str) {
		name=normalize_name(str);
	}
	const std::string& subprogram::get_name() const {
		return name;
	}
	byte subprogram::get_argc() const {
		return argc;
	}
	size_t subprogram::get_codelen() const {
		return len;
	}
	const byte* subprogram::access_code() const {
		return code;
	}

	int subprogram::get_int(size_t& program_counter) const {
		if(program_counter + 4 >= len) throw bytecode::underflow();
		int Result = ((code[program_counter] & 0xff) << 24) | ((code[program_counter+1] & 0xff) << 16) | ((code[program_counter+2] & 0xff) << 8) | (code[program_counter+3] & 0xff);
		program_counter+=4;
		return Result;
	}

	byte subprogram::get_byte(size_t& program_counter) const {
		if(program_counter >= len) throw bytecode::underflow();
		return code[program_counter++];
	}


	std::string subprogram::get_string(size_t& program_counter) const {
		std::string Result;
		Result.reserve(16);
		char c;
		while(code[program_counter] != 0) {
			if(program_counter >= len) throw bytecode::underflow();
			c = code[program_counter];
			Result.append( 1,c );
			++program_counter;
		}
		++program_counter;
		return Result;
	}

	void subprogram::get_bytecode(byte*& Result, size_t& length) {
		length =   1                 // proc marker
		         + get_name().size() // characters of the name
		         + 1                 // null
		         + 1                 // argc
		         + 4                 // codelen
		         + len;              // code
		Result = new byte[length];

		Result[0] = static_cast<byte>(bytecode::PROC);  // proc marker
		size_t c = 1;
		for(size_t i=0; i<get_name().size(); ++i, ++c) {     // name
			Result[c] = get_name()[i];
		}
		Result[c++] = 0;
		Result[c++] = static_cast<byte>(argc);
		for(size_t i=0; i<4; ++i) {
 			Result[c+3-i] = (len >> (8*i));	// bigendian codelen
		}
		c+=4;
		for(size_t i=0; i<len; ++i, ++c) {
			Result[c] = code[i];
		}
	}


	void subprogram::print_bytecode(std::ostream& out) {
		// proc marker, followed by canonical name closed by a zero.
		out << static_cast<byte>(bytecode::PROC) << get_name() << '\0'
		//argc in a single byte
			<< static_cast<byte>(argc);

		//then the actual program
		for(size_t i=0; i<len; ++i) out << code[i];
	}

	void subprogram::print_assembly(std::ostream& out) {
		out << "PROCEDURE " << get_name() << " (" << (int)argc << ")" << std::endl
			<< std::setw(4) << "line" << std::setw(6) << "code" << std::setw(10)  << "arg" << " followup" << std::endl;

		size_t pc = 0;
		size_t line = 0;
		Instruction opcode;
		while(pc<len) {
			opcode = static_cast<Instruction>(get_byte(pc));
			out << std::setw(4) << line << std::setw(6) << static_cast<int>(opcode) << std::setw(10);

			if(arglen(opcode) == 4) {
				out << get_int(pc);
			}
			else if(arglen(opcode) == 1) {
				out << (int)get_byte(pc);
			}
			else out << " ";

			if(has_followup(opcode)) {
				out << " " << get_string(pc);
			}

			out << std::endl;
			++line;
		}
	}
}
