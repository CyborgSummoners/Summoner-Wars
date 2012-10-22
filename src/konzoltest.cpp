#include <iostream>
#include "terminal.hpp"

int main() {

	std::string cmd;
	sum::Terminal terminal;
	while(true) {
		getline(std::cin, cmd);
		std::cout << terminal.command(cmd);
	}

	return 0;
}
