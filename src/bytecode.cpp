#include "bytecode.hpp"
#include "interpreter.hpp"


namespace bytecode {
	bool has_argument(Instruction i) {
		switch(i) {
			case PUSH:
			case PSHB:
			case ISP:
			case DSP:
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

	bool has_followup(Instruction i) {
		return i==CALL;
	}


	int get_interrupt_id(const std::string& str) {
		return sum::Interpreter::get_interrupt_id(str);
	}


 	std::string subprogram::normalize_name(std::string str) {
		if( str.size() > 15 ) str.resize(15);
		for(size_t i=0; i<str.length(); ++i) str[i] = toupper(str[i]);
		return str;
	}

	subprogram::subprogram(std::string name, byte* code, size_t len) : name(normalize_name(name)), code(code), len(len) {}

	void subprogram::set_name(const std::string& str) {
		name=normalize_name(str);
	}
	const std::string& subprogram::get_name() const {
		return name;
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
}
