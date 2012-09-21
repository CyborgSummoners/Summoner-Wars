#include <iostream>
#include <fstream>
#include "summparse.h"
#include "bytecode.hpp"
#include <FlexLexer.h>

int Parser::lex() {
	int ret = lexer->yylex();
	d_loc__.first_line = lexer->lineno();
	d_val__.name = new std::string(lexer->YYText());
	return ret;
}


void Parser::second_pass(std::vector<codeline>& code) {
	if(code.size() < 1) return;

	// fixes
	for(size_t i=0; i<code.size(); ++i) {
		// unite uninterrupted stack pointer movement blocks
		if( code[i].opcode == ISP || code[i].opcode == DSP ) {
			for( size_t k=i+1; k<code.size() && code[k].opcode == code[i].opcode && code[k].label == 0; ++k ) {
				code[k].opcode = NOP;
				code[i].argument += code[k].argument;
			}
		}

		// push ahead labels of NOPs
		if( code[i].opcode==NOP && code[i].label!=0 && i+1<code.size()) {
			code[i+1].label = code[i].label;
			code[i].label = 0;
		}
	}

	// calculate real line numbers (not counting labelless NOPs)
	for(size_t i=0, line=0; i<code.size(); ++i) {
		if( !(code[i].opcode==NOP && code[i].label==0) ) code[i].line_no = line++;
	}
}


void Parser::assemble(std::vector<codeline>& code) {
	// build labelmap
	std::map<uint32_t, int> labelmap;
	for(size_t i=0; i<code.size(); ++i) {
		if(code[i].label != 0) labelmap.insert( std::make_pair(code[i].label, code[i].line_no) );
	}

	// replace jump <label>s with jump <line_no>s
	for(size_t i=0; i<code.size(); ++i) {
		if(code[i].opcode >= JMP && code[i].opcode <=JMPFALSE) {
			code[i].argument = labelmap[static_cast<uint32_t>(code[i].argument)];
		}
	}
}


int main() {
	std::ifstream source("script-samples/cgtest.summ");
	Parser parser(new yyFlexLexer(&source, &std::cout));
	bool error=parser.parse();
	source.close();
	return error;
}
