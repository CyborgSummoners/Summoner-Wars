#include "bytecode.hpp"


namespace bytecode {
	bool has_argument(Instruction i) {
		switch(i) {
			case PUSH:
			case ISP:
			case DSP:
			case FETCH_X:
			case STORE_X:
			case JMP:
			case JMPTRUE:
			case JMPFALSE:
			case DELAY:
				return true;
			default:
				return false;
		}
	}

	bool has_followup(Instruction i) {
		return i==CALL;
	}
}
