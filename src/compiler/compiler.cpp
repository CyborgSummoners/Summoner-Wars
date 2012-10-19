#include "../bytecode.hpp"
#include "summparse.h"

Parser::Parser(std::istream& in, std::ostream& out, action act) : act(act), lexer( new Lexer(in, out) ), varnum(0) {
}
Parser::~Parser() {
	delete lexer;
}

int Parser::lex() {
	return lexer->yylex(&d_val__, &d_loc__);
}

int Lexer::yylex(Parser::STYPE__* d_val, Parser::LTYPE__* d_loc) {
	this->d_val = d_val;
	this->d_loc = d_loc;
	int ret = yylex();
	d_loc->first_line = lineno();
	return ret;
}


uint32_t Parser::get_value(const std::string& str) {
	//max signed int: 2^31-1 = 2,147,483,647
	static uint32_t top=214748364; 	//2,147,483,647 / 10

	uint32_t Result=0;
	bool warn=false;

	for(size_t i=0; i<str.size(); ++i) {
		if( !warn && (Result>top || ( Result==top && (str[i]-48)>7) ) ) {
			warning("overflow: numeric constant too large to be represented in a four byte signed integer.");
			warn=true;
		}
		Result=Result*10 + (str[i]-48);
	}
	return Result;
}


uint32_t Parser::gen_label() {
	static uint32_t label = 0;
	return ++label;
}


size_t Parser::gen_varnum() {
	return varnum++;
}

void Parser::second_pass(codelines& code) {
	if(code.size() < 1) return;

	// fixes
	size_t vars = 0;
	for(code_iterator it=code.begin(), it2; it!=code.end(); ++it) {
		// gather all space reservations into one big block.
		if( it->opcode == RSRV ) {
			++vars;
			it->opcode = NOP;
		}

		// push ahead labels of NOPs
		it2 = it;
		++it2;
		if( it->opcode==NOP && it->label!=0 && (it+1)!=code.end() && (it+1)->label==0 ) {
			(it+1)->label = it->label;
			it->label = 0;
		}
	}

	//put all reservations to the front:
	if(vars > 0) code.insert(code.begin(), codeline(RSRV, vars));

	// calculate real line numbers (not counting labelless NOPs)
	size_t line=0;
	for(code_iterator it=code.begin(); it!=code.end(); ++it) {
		if( !(it->opcode==NOP && it->label==0) ) it->line_no = line++;
	}
}


void Parser::assemble(codelines& code, byte*& Result, size_t& length) {
	// count length (in bytes) and build labelmap
	std::map<uint32_t, int> labelmap;
	length = 0;
	for(code_iterator it=code.begin(); it!=code.end(); ++it) {
		if(it->label != 0) labelmap.insert( std::make_pair(it->label, length) );

		if( !(it->opcode == NOP && it->label==0) ) {
			++length;
			length+=arglen(it->opcode);
			if( has_followup(it->opcode) ) length+=it->followup.length()+1;
		}
	}

	// replace jump <label>s with jump <program_counter>s
	for(code_iterator it=code.begin(); it!=code.end(); ++it) {
		if(it->opcode >= JMP && it->opcode <=JMPFALSE) {
			it->argument = labelmap[static_cast<uint32_t>(it->argument)];
		}
	}

	Result = new byte[length];
	size_t len=0;
	for(code_iterator it=code.begin(); it!=code.end(); ++it) {
		if( it->opcode == NOP && it->label==0 ) continue;	//cull labelless NOPs

		Result[len] = it->opcode;
		++len;

		if(arglen(it->opcode) == 4) {
			for(size_t k=0; k<4; ++k) {
				Result[len+3-k] = (it->argument >> (8*k));	// bigendian
			}
			len+=4;
		}
		else if(arglen(it->opcode) == 1) {
			Result[len] = static_cast<byte>(it->argument);
			++len;
		}
		// itt dobhatna kivételt, ha olyannal találkozik, amit nem ismer

		if( has_followup(it->opcode) ) {
			for(size_t k=0; k<it->followup.length(); ++k, ++len) {
				Result[len] = it->followup[k];
			}
			Result[len] = 0;
			++len;
		};
	}
}

void Parser::reset() {
	varnum = 0;
	symtab.clear();
}

bool var::is(type typ) const {
	return (this->typ==any || subtypeof(typ, this->typ));
}

bool expression::is(type typ) const {
	return (this->typ==any || subtypeof(typ, this->typ));
}
