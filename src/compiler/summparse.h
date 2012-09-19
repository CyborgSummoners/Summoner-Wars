// Generated by Bisonc++ V2.4.8 on Tue, 18 Sep 2012 16:33:31 +0200
// then heavily modified. DO NOT try to delete and regen.

#ifndef Parser_h_included
#define Parser_h_included

// $insert baseclass
#include "summparsebase.h"
#include "semantics.hpp"
#include <map>
#include <stdint.h>


#undef Parser

class yyFlexLexer;

class Parser: public ParserBase
{
	private:
		yyFlexLexer* lexer;

	public:
		Parser(yyFlexLexer* lexer) : lexer(lexer){}

        
    public:
        int parse();

    private:
        void error(char const *msg);    // called on (syntax) errors
        void warning(char const *msg);
        int lex();                      // returns the next token from the
                                        // lexical scanner. 
        void print();                   // use, e.g., d_token, d_loc

    // support functions for parse():
        void executeAction(int ruleNr);
        void errorRecovery();
        int lookup(bool recovery);
        void nextToken();

		std::map<std::string, var> symtab;	// symbol table

		uint32_t gen_label() {
			static int label = 0;
			return ++label;
		}

		uint32_t get_value(const std::string& str) {
			//maxint: 4,294,967,295
			static uint32_t top=429496729; 	//4,294,967,295 / 10

			uint32_t Result=0;
			bool warn=false;

			for(size_t i=0; i<str.size(); ++i) {
				if( !warn && (Result>top || ( Result==top && (str[i]-48)>5) ) ) {
					warning("overflow: numeric constant too large to be represented in a four byte unsigned integer.");
					warn=true;
				}
				Result=Result*10 + (str[i]-48);
			}

			return Result;
		}
};

inline void Parser::error(char const *msg) {
	std::cerr << d_loc__.first_line << ": " << msg;
}

inline void Parser::warning(char const *msg) {
	std::cerr << d_loc__.first_line << ": " << msg;
}




// $insert lex

inline void Parser::print()      // use d_token, d_loc
{}


#endif
