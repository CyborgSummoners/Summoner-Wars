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

integer                 return Parser::T_INTEGER;
boolean                 return Parser::T_BOOLEAN;

if                      return Parser::K_IF;
then                    return Parser::K_THEN;
else                    return Parser::K_ELSE;

while                   return Parser::K_WHILE;
loop                    return Parser::K_LOOP;

true                    return Parser::L_TRUE;
false                   return Parser::L_FALSE;

and                     return Parser::OP_AND;
or                      return Parser::OP_OR;
not                     return Parser::OP_NOT;

;                       return Parser::SEMICOLON;
:                       return Parser::COLON;
,                       return Parser::COMMA;
\(                      return Parser::T_OPEN;
\)                      return Parser::T_CLOSE;
:=                      return Parser::OP_ASSIGNMENT;
=                       return Parser::OP_EQUALITY;
\<                      return Parser::OP_LESS_THAN;
>                       return Parser::OP_GREATER_THAN;
\+                      return Parser::OP_PLUS;
-                       return Parser::OP_MINUS;
\*                      return Parser::OP_MULTIPLY;
div                     return Parser::OP_DIV;
mod                     return Parser::OP_MOD;

[a-zA-Z][a-zA-Z0-9_]*   return Parser::IDENTIFIER;

[0-9]+                  return Parser::L_INTEGER;

[ \n\t]

.                       {
	std::cerr << lineno()<<": lexical error: invalid character ('" << YYText() << "')." << std::endl;
	exit(1);
						}

%%
