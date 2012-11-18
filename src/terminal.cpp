#include "terminal.hpp"

namespace sum {
	std::string Terminal::command(const std::string& input) {
		return input+"\n";
	}	
	
	std::string Terminal::get_working_directory() {
		return "/";
	}
}
