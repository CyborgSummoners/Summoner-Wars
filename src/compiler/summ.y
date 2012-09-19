%lsp-needed
%baseclass-preinclude "semantics.hpp"

%type <typ> type
%type <exp> constant
%type <exp> exp

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
		std::cout << *$1 << " declared" << std::endl;
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
	for(size_t i=0; i<$3->code.size();++i) {
		$3->code[i].print();
	}
}
;

exp:
IDENTIFIER {
	std::cout << "exp -> identifier" << std::endl;

		if(symtab.count(*$1) > 0) { //does it exist?
			if(symtab[*$1].read == 0) { //is it read for the first time?
				symtab[*$1].read = d_loc__.first_line;
				if(symtab[*$1].read < symtab[*$1].writ) { //was it written?
					std::stringstream ss;
					ss << "variable " << *$1 << " doesn't seem to be initialized when first read";
					warning(ss.str().c_str());
				}
			}
			$$ = new expression(symtab[*$1].typ);
			$$->code.push_back( codeline("", FETCH, symtab[*$1].num) );
		} else {
			std::stringstream ss;
			ss << "Variable '" << *$1 << "' undeclared." << std::endl;
			error(ss.str().c_str());
		}
		delete $1;
	}
| T_OPEN exp T_CLOSE {
	std::cout << "exp -> ( exp )" << std::endl;
	$$ = $2;
	}
| constant {
	std::cout << "exp -> constant" << std::endl;
	$$ = $1;
	}
| exp OP_PLUS exp {
	std::cout << "exp -> exp + exp" << std::endl;
		//both operands must be integers
		if($1->typ == $3->typ && $1->typ == integer) {
			$$ = new expression($1->typ);

			$$->code.insert( $$->code.begin(), $1->code.begin(), $1->code.end() );	// első operandus
			$$->code.insert( $$->code.end(), $3->code.begin(), $3->code.end() );	// második operandus

			$$->code.push_back( codeline("", ADDI, 0) );

			delete $1;
			delete $3;
		}
		else {
			std::stringstream ss;
			ss << "Type mismatch (both operands must be integers)" << std::endl;
			error(ss.str().c_str());
			$$ = $1;
			delete $3;
		}
	}
| exp OP_MINUS exp {
	std::cout << "exp -> exp - exp" << std::endl;

		//both operands must be integers
		if($1->typ == $3->typ && $1->typ == integer) {
			$$ = new expression($1->typ);

			$$->code.insert( $$->code.begin(), $1->code.begin(), $1->code.end() );	// első operandus
			$$->code.insert( $$->code.end(), $3->code.begin(), $3->code.end() );	// második operandus

			$$->code.push_back( codeline("", SUBI, 0) );

			delete $1;
			delete $3;
		}
		else {
			std::stringstream ss;
			ss << "Type mismatch (both operands must be integers)" << std::endl;
			error(ss.str().c_str());
			$$ = $1;
			delete $3;
		}
	}
| exp OP_DIV exp {
	std::cout << "exp -> exp div exp" << std::endl;

		//both operands must be integers
		if($1->typ == $3->typ && $1->typ == integer) {
			$$ = new expression($1->typ);

			$$->code.insert( $$->code.begin(), $1->code.begin(), $1->code.end() );	// első operandus
			$$->code.insert( $$->code.end(), $3->code.begin(), $3->code.end() );	// második operandus

			$$->code.push_back( codeline("", DIVI, 0) );

			delete $1;
			delete $3;
		}
		else {
			std::stringstream ss;
			ss << "Type mismatch (both operands must be integers)" << std::endl;
			error(ss.str().c_str());
			$$ = $1;
			delete $3;
		}
	}
| exp OP_MOD exp {
	std::cout << "exp -> exp mod exp" << std::endl;

		//both operands must be integers
		if($1->typ == $3->typ && $1->typ == integer) {

			$$ = new expression($1->typ);

			$$->code.insert( $$->code.begin(), $1->code.begin(), $1->code.end() );	// első operandus
			$$->code.insert( $$->code.end(), $3->code.begin(), $3->code.end() );	// második operandus

			$$->code.push_back( codeline("", MODI, 0) );

			delete $1;
			delete $3;
		}
		else {
			std::stringstream ss;
			ss << "Type mismatch (both operands must be integers)" << std::endl;
			error(ss.str().c_str());
			$$ = $1;
			delete $3;
		}
	}
| exp OP_MULTIPLY exp {
	std::cout << "exp -> exp * exp" << std::endl;

		//both operands must be integers
		if($1->typ == $3->typ && $1->typ == integer) {
			$$ = new expression($1->typ);

			$$->code.insert( $$->code.begin(), $1->code.begin(), $1->code.end() );	// első operandus
			$$->code.insert( $$->code.end(), $3->code.begin(), $3->code.end() );	// második operandus

			$$->code.push_back( codeline("", MULI, 0) );

			delete $1;
			delete $3;
		}
		else {
			std::stringstream ss;
			ss << "Type mismatch (both operands must be integers)" << std::endl;
			error(ss.str().c_str());
			$$ = $1;
			delete $3;
		}
	}
| exp OP_AND exp {
	std::cout << "exp -> exp AND exp" << std::endl;

		//both operands must be booleans
		if($1->typ == $3->typ && $1->typ == boolean) {

			$$ = new expression($1->typ);

			$$->code.insert( $$->code.begin(), $1->code.begin(), $1->code.end() );	// első operandus
			$$->code.insert( $$->code.end(), $3->code.begin(), $3->code.end() );	// második operandus

			$$->code.push_back( codeline("", AND, 0) );

			delete $1;
			delete $3;
		}
		else {
			std::stringstream ss;
			ss << "Type mismatch (both operands must be booleans)" << std::endl;
			error(ss.str().c_str());
			$$ = $1;
			delete $3;
		}
	}
| exp OP_OR exp {
	std::cout << "exp -> exp OR exp" << std::endl;

		//both operands must be booleans
		if($1->typ == $3->typ && $1->typ == boolean) {

			$$ = new expression($1->typ);

			$$->code.insert( $$->code.begin(), $1->code.begin(), $1->code.end() );	// első operandus
			$$->code.insert( $$->code.end(), $3->code.begin(), $3->code.end() );	// második operandus

			$$->code.push_back( codeline("", OR, 0) );

			delete $1;
			delete $3;
		}
		else {
			std::stringstream ss;
			ss << "Type mismatch (both operands must be booleans)" << std::endl;
			error(ss.str().c_str());
			$$ = $1;
			delete $3;
		}
	}
| OP_NOT exp {
	std::cout << "exp -> NOT exp" << std::endl;
	$$ = $2;
	$$->code.push_back( codeline("", NOT, 0) );
	}
| exp OP_EQUALITY exp {
	std::cout << "exp -> exp = exp" << std::endl;

		//operands must be of the same type 
		if($1->typ == $3->typ) {

			$$ = new expression(boolean);

			$$->code.insert( $$->code.begin(), $1->code.begin(), $1->code.end() );	// első operandus
			$$->code.insert( $$->code.end(), $3->code.begin(), $3->code.end() );	// második operandus

			$$->code.push_back( codeline("", EQ, 0) );

			delete $1;
			delete $3;
		}
		else {
			std::stringstream ss;
			ss << "Type mismatch (operands must be of the same type)" << std::endl;
			error(ss.str().c_str());
			$$ = $1;
			delete $3;
		}
	}
| exp OP_LESS_THAN exp {
	std::cout << "exp -> exp < exp" << std::endl;

		//both operands must be integers
		if($1->typ == $3->typ && $1->typ == integer) {
			$$ = new expression(boolean);

			$$->code.insert( $$->code.begin(), $1->code.begin(), $1->code.end() );	// első operandus
			$$->code.insert( $$->code.end(), $3->code.begin(), $3->code.end() );	// második operandus

			$$->code.push_back( codeline("", LESS, 0) );

			delete $1;
			delete $3;
		}
		else {
			std::stringstream ss;
			ss << "Type mismatch (both operands must be integers)" << std::endl;
			error(ss.str().c_str());
			$$ = $1;
			delete $3;
		}
	}
| exp OP_GREATER_THAN exp {
	std::cout << "exp -> exp > exp" << std::endl;

		//both operands must be integers
		if($1->typ == $3->typ && $1->typ == integer) {
			$$ = new expression(boolean);

			$$->code.insert( $$->code.begin(), $1->code.begin(), $1->code.end() );	// első operandus
			$$->code.insert( $$->code.end(), $3->code.begin(), $3->code.end() );	// második operandus

			$$->code.push_back( codeline("", GREATER, 0) );

			delete $1;
			delete $3;
		}
		else {
			std::stringstream ss;
			ss << "Type mismatch (both operands must be integers)" << std::endl;
			error(ss.str().c_str());
			$$ = $1;
			delete $3;
		}
	}
;

constant:
L_TRUE {
	$$ = new expression(boolean);
	$$->code.push_back( codeline("", PUSH, 1) );
	}
| L_FALSE {
	$$ = new expression(boolean);
	$$->code.push_back( codeline("", PUSH, 0) );
	}
| L_INTEGER {
	$$ = new expression(integer);
	$$->code.push_back( codeline("", PUSH, 5) ); //FIXME proper value
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
