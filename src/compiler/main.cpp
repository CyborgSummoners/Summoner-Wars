#include <iostream>
#include <fstream>
#include <FlexLexer.h>
#include "summparse.h"
#include "../interpreter.hpp"


int main() {
	//interpreter demo
	std::ifstream source("script-samples/cgtest.summ");
	yyFlexLexer* lexer = new yyFlexLexer(&source, &std::cout);
	Parser parser(lexer);
	bool error=parser.parse();
	source.close();
	delete lexer;

	if(error) return error;
/*
	sum::Interpreter interpreter;

	interpreter.register_subprogram(parser.subprograms[0]);

	interpreter.execute(parser.subprograms[0].get_name());
*/
	return error;
}
