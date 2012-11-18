#include "terminal.hpp"

namespace sum {
	namespace {
		std::string whitespace = " \t\n\r";	
	}
	
	std::string Terminal::command(std::string input) {
		// trim left
		size_t s = input.find_first_not_of(whitespace);
		if(std::string::npos != s) input.substr(s).swap(input);

		if(input.empty()) return "";

		// get command -- everything before the first space.
		s = input.find_first_of(whitespace);
		std::string command = input.substr( 0, s );

		if("pwd" == command) {
			return get_working_directory()+"\n";
		}

		return command+": command not found\n";
	}	
	
	std::string Terminal::get_working_directory() {
		return "/";
	}
}
