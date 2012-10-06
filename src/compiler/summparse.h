// Generated by Bisonc++ V2.4.8 on Tue, 18 Sep 2012 16:33:31 +0200
#ifndef COMPILER_PARSER_H
#define COMPILER_PARSER_H 1

#include "../bytecode.hpp"
#include "summparsebase.h"
#include "compiler.hpp"
#include <map>
#include <stdint.h>

#ifndef yyFlexLexerOnce
#include <FlexLexer.h>
#endif

#undef Parser


class Lexer;

class Parser : public ParserBase {
	// more or less temporary:
	public:
		enum action {FULL=0, ASSEMBLED, BYTECODE, NOINPUT, FILE_ERROR, MALFORMED_ARGS};
	private:
		action act;


	private:
		Lexer* lexer;
		size_t varnum;

	public:
		std::vector<bytecode::subprogram> subprograms;	// list of parsed subprograms
		std::map<std::string, var> symtab;	// symbol table of variables

	public:
		Parser(std::istream& in, std::ostream& out, action act=FULL);
		~Parser();

    public:
        int parse();	// starts the parsing

		static void second_pass(codelines& code);	// cleanup & optimize code a bit.
		static void assemble(codelines& code, byte*& Result, size_t& length);	// assemble code into actual bytecode.

    private:
        void error(char const *msg);    // called on (syntax) errors
        void warning(char const *msg);
        int lex();                      // returns the next token from the lexical scanner.
		void print();                   // use, e.g., d_token, d_loc

		// support functions for parse():
		void executeAction(int ruleNr);
		void errorRecovery();
		int lookup(bool recovery);
		void nextToken();

		//utility
		uint32_t gen_label();
		size_t gen_varnum();
		uint32_t get_value(const std::string& str);

		void reset();
};


class Lexer : public yyFlexLexer {
	private:
		Parser::STYPE__* d_val;
		Parser::LTYPE__* d_loc;
		Lexer(std::istream& in, std::ostream& out) : yyFlexLexer(&in, &out) {}
		int yylex(Parser::STYPE__* d_val, Parser::LTYPE__* d_loc);
		int yylex(); // implemented by flex

	friend class Parser;
};


inline void Parser::error(char const *msg) {
	std::cerr << d_loc__.first_line << ": " << msg;
}

inline void Parser::warning(char const *msg) {
	std::cerr << d_loc__.first_line << ": " << msg;
}

inline void Parser::print() {
}

#endif
