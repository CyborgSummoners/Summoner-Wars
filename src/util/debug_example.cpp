//#define DEBUG_MACROS_ENABLED 1
#include "debug.hpp"
#include <iostream>

int main() {
	int i=74;
	debug_var(i);
	dout << "Have you seen the debug message about the variable i?" << std::endl;
	if(i == 74) std::cout << "<_<" << std::endl;
}
