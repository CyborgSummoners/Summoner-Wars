#include "bytecode.hpp"


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


 	std::string subprogram::normalize_name(std::string str) {
		if( str.size() > 15 ) str.resize(15);
		for(size_t i=0; i<str.length(); ++i) str[i] = toupper(str[i]);
		return str;
	}

	subprogram::subprogram(std::string name, byte* code, size_t len) : name(name), code(code), len(len) {}

	void subprogram::set_name(const std::string& str) {
		name=normalize_name(str);
	}
	std::string subprogram::get_name() const {
		return name;
	}

	int subprogram::get_int(size_t& startpos) const {
		int Result = ((code[startpos] & 0xff) << 24) | ((code[startpos+1] & 0xff) << 16) | ((code[startpos+2] & 0xff) << 8) | (code[startpos+3] & 0xff);
		startpos+=4;
		return Result;
	}

	const std::map<std::string, builtin_call> builtin_call::init_mapping() {
		std::map<std::string, builtin_call> Result;
		Result.insert( std::make_pair("PRINT", builtin_call(0, 1)) );
		return Result;
	}
	const std::map<std::string, builtin_call> builtin_call::mapping=init_mapping();

	bool builtin_call::call_exists(const std::string& name) {
		return mapping.find(name) != mapping.end();
	}
	builtin_call builtin_call::get_call(const std::string& name) {
		return mapping.find(name)->second;
	}
}
