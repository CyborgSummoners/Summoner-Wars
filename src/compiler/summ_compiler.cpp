#include <iostream>
#include <fstream>
#include "summparse.h"
#include <FlexLexer.h>

int Parser::lex() {
	int ret = lexer->yylex();
	d_loc__.first_line = lexer->lineno();
	d_val__.name = new std::string(lexer->YYText());
	return ret;
}

int main() {
	std::ifstream source("script-samples/cgtest.summ");
	Parser parser(new yyFlexLexer(&source, &std::cout));
	bool error=parser.parse();
	source.close();
	return error;
}
