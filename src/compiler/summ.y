%lsp-needed
%baseclass-preinclude <iostream>

%token IDENTIFIER
%token K_PROCEDURE
%token K_IS
%token K_BEGIN
%token K_END
%token K_IF
%token K_THEN
%token K_ELSE
%token K_WHILE
%token K_LOOP

%token OP_ASSIGNMENT

%token COLON
%token SEMICOLON
%token COMMA

%token T_OPEN
%token T_CLOSE

%token L_TRUE
%token L_FALSE

%left OP_AND
%left OP_OR
%right OP_NOT

%left OP_EQUALITY
%left OP_LESS_THAN
%left OP_GREATER_THAN

%left OP_PLUS
%left OP_MINUS
%left OP_MULTIPLY
%left OP_DIV
%left OP_MOD

%token L_INTEGER

%token K_NULL

%%

start:
procedure {
	std::cout << "start -> procedure" << std::endl;
	}
;

procedure:
signature declarations proc_body {
	std::cout << "procedure -> signature proc_body" << std::endl;
	}
;

signature:
K_PROCEDURE IDENTIFIER K_IS {
	std::cout << "signature -> K_PROCEDURE IDENTIFIER K_IS" << std::endl;
	}
;

declarations:
//epszilon
|
declarations decl {
	std::cout << "declarations -> declarations decl" << std::endl;
	}
;

decl:
IDENTIFIER COLON IDENTIFIER {
	std::cout << "decl -> identifier colon type" << std::endl;
	}
;

proc_body:
K_BEGIN statements K_END {
	std::cout << "proc_body -> K_BEGIN K_NULL K_END" << std::endl;
	}
|
K_BEGIN statements K_END IDENTIFIER {
	std::cout << "proc_body -> K_BEGIN K_NULL K_END IDENTIFIER" << std::endl;
	}
;


statements:
statements statement {
	std::cout << "statements -> statements statement" << std::endl;
	}
|
statement {
	std::cout << "statements -> statement" << std::endl;
	}
;

statement:
K_WHILE exp K_LOOP statements K_END K_LOOP {
	std::cout << "statement -> while exp loop statements end loop" << std::endl;
	}
| conditional {
	std::cout << "statement -> conditional" << std::endl;
	}
| proc_call {
	std::cout << "statement -> proc_call" << std::endl;
	}
| assignment {
	std::cout << "statements -> assignment" << std::endl;
	}
| K_NULL {
	std::cout << "statements -> null" << std::endl;
	}
;

conditional:
K_IF exp K_THEN statements K_END K_IF {
	std::cout << "conditional -> if exp then statements end if" << std::endl;
	}
| K_IF exp K_THEN statements K_ELSE statements K_END K_IF {
	std::cout << "conditional -> if exp then statements else statements end if" << std::endl;
	}
;

proc_call:
	IDENTIFIER T_OPEN exp T_CLOSE {
		std::cout << "proc_call -> identifier(exp)" << std::endl;
	}
;

assignment:
IDENTIFIER OP_ASSIGNMENT exp {
	std::cout << "assignment -> identifier OP_ASSIGNMENT exp" << std::endl;
}
;

exp:
IDENTIFIER {
	std::cout << "exp -> identifier" << std::endl;
	}
| T_OPEN exp T_CLOSE {
	std::cout << "exp -> ( exp )" << std::endl;
	}
| constant {
	std::cout << "exp -> constant" << std::endl;
	}
| exp OP_PLUS exp {
	std::cout << "exp -> exp + exp" << std::endl;
	}
| exp OP_MINUS exp {
	std::cout << "exp -> exp - exp" << std::endl;
	}
| exp OP_DIV exp {
	std::cout << "exp -> exp / exp" << std::endl;
	}
| exp OP_MOD exp {
	std::cout << "exp -> exp % exp" << std::endl;
	}
| exp OP_MULTIPLY exp {
	std::cout << "exp -> exp * exp" << std::endl;
	}
| exp OP_AND exp {
	std::cout << "exp -> exp AND exp" << std::endl;
	}
| exp OP_OR exp {
	std::cout << "exp -> exp OR exp" << std::endl;
	}
| OP_NOT exp {
	std::cout << "exp -> NOT exp" << std::endl;
	}
| exp OP_EQUALITY exp {
	std::cout << "exp -> exp = exp" << std::endl;
	}
| exp OP_LESS_THAN exp {
	std::cout << "exp -> exp < exp" << std::endl;
	}
| exp OP_GREATER_THAN exp {
	std::cout << "exp -> exp > exp" << std::endl;
	}
;

constant:
L_TRUE {
	std::cout << "constant -> TRUE" << std::endl;
	}
| L_FALSE {
	std::cout << "constant -> FALSE" << std::endl;
	}
| L_INTEGER {
	std::cout << "constant -> integer" << std::endl;
	}
;
