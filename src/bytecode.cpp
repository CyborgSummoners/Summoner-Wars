#include "bytecode.hpp"
#include "interpreter.hpp"
#include <iomanip>
#include <iostream>

namespace bytecode {
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

	subprogram::subprogram(std::string name, byte argc, byte* code, size_t len, bool retval) : name(normalize_name(name)), argc(argc), retval(retval), code(code), len(len) {}

	void subprogram::set_name(const std::string& str) {
		name=normalize_name(str);
	}
	const std::string& subprogram::get_name() const {
		return name;
	}
	byte subprogram::get_argc() const {
		return argc;
	}

	int subprogram::get_int(size_t& program_counter) const {
		int Result = ((code[program_counter] & 0xff) << 24) | ((code[program_counter+1] & 0xff) << 16) | ((code[program_counter+2] & 0xff) << 8) | (code[program_counter+3] & 0xff);
		program_counter+=4;
		return Result;
	}

	byte subprogram::get_byte(size_t& program_counter) const {
		return code[program_counter++];
	}


	std::string subprogram::get_string(size_t& program_counter) const {
		std::string Result;
		Result.reserve(16);
		char c;
		while(code[program_counter] != 0) {
			c = code[program_counter];
			Result.append( 1,c );
			++program_counter;
		}
		++program_counter;
		return Result;
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
