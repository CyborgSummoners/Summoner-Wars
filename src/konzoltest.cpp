#include <iostream>
#include <string>
#include "terminal.hpp"

int main() {
	std::string user = "Summoner";
	std::string cmd;
	sum::Terminal terminal;
	while(true) {
		std::cout << user << ":" << terminal.get_working_directory() << "$ ";
		getline(std::cin, cmd);
		std::cout << terminal.command(cmd);
	}

	return 0;
}
