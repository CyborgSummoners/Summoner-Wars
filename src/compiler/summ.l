%option noyywrap yylineno c++ yyclass="Lexer"

%{
#include "summparse.h"
%}


%x STR
%%

		char str_buf[2048];
		char* str_buf_ptr;


--.*                    // comment
procedure               return Parser::K_PROCEDURE;
function                return Parser::K_FUNCTION;
is                      return Parser::K_IS;
end                     return Parser::K_END;
return                  return Parser::K_RETURN;
null                    return Parser::K_NULL;

self                    return Parser::K_SELF;

if                      return Parser::K_IF;
then                    return Parser::K_THEN;
else                    return Parser::K_ELSE;
elsif                   return Parser::K_ELSIF;

while                   return Parser::K_WHILE;
loop                    return Parser::K_LOOP;

true                    return Parser::L_TRUE;
false                   return Parser::L_FALSE;

and                     return Parser::OP_AND;
or                      return Parser::OP_OR;
not                     return Parser::OP_NOT;

\.                      return Parser::OP_MEMBER;
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
&                       return Parser::OP_CONCAT;

\"                      {	// new string. pointer points at the start of the buffer.
                            BEGIN(STR);
                            str_buf_ptr = str_buf;
                        }
<STR>{
    \"                  {	// end of string, go back to initial state
                            BEGIN(INITIAL);
                            *str_buf_ptr = 0;
                            d_val->str = new std::string(str_buf);
                            return Parser::L_STRING;
                        }
    \n                  {
                            BEGIN(INITIAL);
                            std::cerr << lineno() << "Unterminated string literal";
                            return Parser::LEXICAL_ERROR;
						}
    "\\n"                   *str_buf_ptr++ = '\n';
    [^\\\n\"]+          {
                            char* yyptr = yytext;
                            while(*yyptr) *str_buf_ptr++ = *yyptr++;
                        }
}

[a-zA-Z][a-zA-Z0-9_]*   {
                        d_val->str = new std::string(YYText());
                        return Parser::IDENTIFIER;
                        }

[0-9]+                  {
                        d_val->str = new std::string(YYText());
                        return Parser::L_INTEGER;
                        }

[ \n\t]

.                       {	//unknown character.
                        std::cerr << lineno()<<": lexical error: invalid character ('" << YYText() << "')." << std::endl;
                        return Parser::LEXICAL_ERROR;
                        }

%%
