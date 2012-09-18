%option noyywrap c++ yylineno

%{
#include "summparse.h"
%}


%%


--.*                    // comment
procedure               return Parser::K_PROCEDURE;
is                      return Parser::K_IS;
begin                   return Parser::K_BEGIN;
end                     return Parser::K_END;
null                    return Parser::K_NULL;

[a-zA-Z][a-zA-Z0-9_]*   return Parser::IDENTIFIER;

[ \n\t]

.                       {
	std::cerr << lineno()<<": lexical error: invalid character ('" << YYText() << "')." << std::endl;
	exit(1);
						}

%%
