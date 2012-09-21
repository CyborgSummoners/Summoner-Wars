#include <iostream>
#include <fstream>
#include <FlexLexer.h>
#include "summparse.h"


int main() {
	std::ifstream source("script-samples/cgtest.summ");
	Parser parser(new yyFlexLexer(&source, &std::cout));
	bool error=parser.parse();
	source.close();

	for(size_t i=0; i<parser.subprograms.size(); ++i) {
		std::cout << parser.subprograms[i].get_name() << " compiled." << std::endl;
	}

	return error;
}
