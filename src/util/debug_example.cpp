//#define DEBUG_MACROS_ENABLED 1
#include "debug.hpp"
#include <iostream>

int main() {
	int i=74;
	debug_var(i);
	dout << "Have you seen the debug message about the variable i? The i is still " << i << std::endl;
	debugf("The number is %d.\nNow try it again with DEBUG_MACROS_ENABLED undeffed\n", i);
	printf("Tip of the day: you can set the debug macros with the compile option -DDEBUG_MACROS_ENABLED\n", i);
	if(i == 74) std::cout << "<_<" << std::endl;
}
