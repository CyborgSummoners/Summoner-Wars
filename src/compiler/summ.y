%lsp-needed
%baseclass-preinclude "semantics.hpp"

%type <typ> type
%type <exp> constant

%token <name> IDENTIFIER
%token K_PROCEDURE
%token K_IS
%token K_BEGIN
%token K_END
%token K_IF
%token K_THEN
%token K_ELSE
%token K_WHILE
%token K_LOOP

%token T_INTEGER
%token T_BOOLEAN

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

%union {
	std::string* name;
	type* typ;
	expression* exp;
	statement* stmt;
}


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
IDENTIFIER COLON type {
	std::cout << "decl -> identifier colon type" << std::endl;

	if(symtab.count(*$1) > 0) { // does the var exist already?
		std::stringstream ss;
		ss << "Variable '" << *$1 << "' already declared (on line " << symtab[*$1].decl << ")." << std::endl;
		error(ss.str().c_str());
	} else {
		symtab[*$1] = var(d_loc__.first_line, *$3);
	}
	delete $3;
	delete $1;
};

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
K_WHILE exp loop {
	std::cout << "statement -> while exp loop" << std::endl;
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

loop:
K_LOOP statements K_END K_LOOP {
	std::cout << "loop -> loop statements end loop" << std::endl;
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
	$$ = new expression(boolean);
	put_opcode($$->code, PUSH);
	put_dword($$->code, 1);
	}
| L_FALSE {
	$$ = new expression(boolean);
	put_opcode($$->code, PUSH);
	put_dword($$->code, 0);
	}
| L_INTEGER {
	$$ = new expression(integer);
	put_opcode($$->code, PUSH);
	put_dword($$->code, 1); //FIXME proper value
	}
;

type:
T_INTEGER {
	$$ = new type(integer);
	}
| T_BOOLEAN {
	$$ = new type(boolean);
	}
;
