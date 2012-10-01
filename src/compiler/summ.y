%lsp-needed
%baseclass-preinclude "compiler.hpp"

%type <typ> type
%type <exp> constant
%type <exp> exp

%type <stmt> assignment
%type <stmt> conditional
%type <stmt> loop
%type <stmt> proc_call
%type <stmt> statement
%type <stmt> statements
%type <stmt> proc_body
%type <stmt> decl
%type <stmt> declarations
%type <stmt> exp_list
%type <stmt> arguments


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
%token K_DO

%token T_INTEGER T_BOOLEAN

%token OP_ASSIGNMENT

%token COLON SEMICOLON COMMA

%token T_OPEN T_CLOSE

%token L_TRUE L_FALSE

%left OP_AND OP_OR
%right OP_NOT

%left OP_EQUALITY OP_LESS_THAN OP_GREATER_THAN

%left OP_CONCAT

%left OP_PLUS OP_MINUS
%left OP_MULTIPLY OP_DIV OP_MOD

%token<name> L_INTEGER

%token K_NULL

%union {
	std::string* name;
	type* typ;
	expression* exp;
	statement* stmt;
}


%%

start:
procedures {
	if(act == BYTECODE) {
		for(size_t i=0; i<subprograms.size(); ++i) subprograms[i].print_bytecode(std::cout);
	}
	else if(act == ASSEMBLED) {
		for(size_t i=0; i<subprograms.size(); ++i) subprograms[i].print_assembly(std::cout);
	}
};

procedures:
procedures procedure
|
procedure
;

procedure:
K_PROCEDURE IDENTIFIER K_IS declarations proc_body IDENTIFIER SEMICOLON {
	if(*$2 != *$6) {
		std::stringstream ss;
		ss << "Name mismatch. Procedure declared as '" << subprogram::normalize_name(*$2) << "' ends as '" << subprogram::normalize_name(*$6) << "'";
		error(ss.str().c_str());
	}
	else {
		$4->code.insert( $4->code.end(), $5->code.begin(), $5->code.end() );

		second_pass($4->code);

		byte* code = 0;
		size_t length = 0;
		assemble($4->code, code, length);
		symtab.clear();
		subprograms.push_back( subprogram(*$2, code, length) );
	}

	delete $2;
	delete $4;
	delete $5;
	delete $6;
};

declarations:
//epszilon
	{
		$$ = new statement();
	}
|
declarations decl {
	$$ = $1;
	$$->code.insert( $$->code.end(), $2->code.begin(), $2->code.end() );
	delete $2;
	}
;

decl:
IDENTIFIER COLON type SEMICOLON {
	$$ = new statement();

	if(symtab.count(*$1) > 0) { // does the var exist already?
		std::stringstream ss;
		ss << "Variable '" << *$1 << "' already declared (on line " << symtab[*$1].decl << ")." << std::endl;
		error(ss.str().c_str());
	} else {
		symtab.insert( make_pair(*$1, var(d_loc__.first_line, *$3)) );
		$$->code.push_back( codeline(ISP, 1) );	// reserve a single space for the variable. Great Big Idea takes care of the rest.
	}

	delete $3;
	delete $1;
};

proc_body:
K_BEGIN statements K_END {
	$$ = $2;
	}
;


statements:
statements statement SEMICOLON {
	$$ = $1;
	$$->code.insert( $$->code.end(), $2->code.begin(), $2->code.end() );
	}
| statement SEMICOLON {
	$$ = $1;
	}
| error SEMICOLON {
	std::cout << "statements -> error semicolon" << std::endl;
	$$ = new statement();
}
;

statement:
loop {
	$$ = $1;
	}
| conditional {
	$$ = $1;
	}
| proc_call {
	$$ = $1;
	}
| assignment {
	$$ = $1;
	}
| K_NULL {
	$$ = new statement();
	$$->code.push_back( codeline(NOP, 0) );
	}
;

loop:
K_WHILE exp K_LOOP statements K_END K_LOOP {
	$$ = new statement();

	if($2->typ != boolean) {
		error("The condition must be a boolean expression");
	}
	else {
		uint32_t start_label = gen_label();
		uint32_t end_label = gen_label();

		$$->code.push_back( codeline(NOP, 0, start_label) );
		$$->code.insert( $$->code.end(), $2->code.begin(), $2->code.end() );
		$$->code.push_back( codeline(JMPFALSE, end_label) );	// if condition is NOT true, we jump to the end.
		$$->code.insert( $$->code.end(), $4->code.begin(), $4->code.end() );  //loop body
		$$->code.push_back( codeline(JMP, start_label) );	//then jump back to the start
		$$->code.push_back( codeline(NOP, 0, end_label) );
	}

	delete $2;
	delete $4;
	}
;

conditional:
K_IF exp K_THEN statements K_END K_IF {
	$$ = new statement();

	if($2->typ != boolean) {
		error("The condition must be a boolean expression");
	}
	else {
		uint32_t label = gen_label();

		$$->code.insert( $$->code.begin(), $2->code.begin(), $2->code.end() );	//feltétel
		$$->code.push_back( codeline(JMPFALSE, label) );	// if condition is NOT true, we jump to the end.
		$$->code.insert( $$->code.end(), $4->code.begin(), $4->code.end() );
		$$->code.push_back( codeline(NOP, 0, label) );
	}

	delete $2;
	delete $4;
	}
| K_IF exp K_THEN statements K_ELSE statements K_END K_IF {
	$$ = new statement();

	if($2->typ != boolean) {
		error("The condition must be a boolean expression");
	}
	else {
		uint32_t else_label = gen_label();
		uint32_t end_label = gen_label();

		$$->code.insert( $$->code.begin(), $2->code.begin(), $2->code.end() );	//feltétel
		$$->code.push_back( codeline(JMPFALSE, else_label) );	// if condition is NOT true, we jump to the else branch.
		$$->code.insert( $$->code.end(), $4->code.begin(), $4->code.end() ); // true branch
		$$->code.push_back( codeline(JMP, end_label) );	//when finished with the branch, we jump to the end.
		$$->code.push_back( codeline(NOP, 0, else_label) );
		$$->code.insert( $$->code.end(), $6->code.begin(), $6->code.end() );
		$$->code.push_back( codeline(NOP, 0, end_label) );
	}

	delete $2;
	delete $4;
	delete $6;
};

proc_call:
	K_DO IDENTIFIER arguments {
		$$ = $3;

		// is it an interrupt?
		std::string norm=subprogram::normalize_name(*$2);
		int intrpt = get_interrupt_id(norm);
		if( intrpt >= 0 ) {
			$$->code.push_back( codeline(INTERRUPT, intrpt) );
		}
		else {
			$$->code.push_back( codeline(CALL, 0, 0, norm ) );
		}

		delete $2;
	}
;

arguments:
	//epszilon
	{
	$$ = new statement();
	}
| T_OPEN exp_list T_CLOSE {
	$$ = new statement();
	$$->code.insert($$->code.begin(), $2->code.begin(), $2->code.end());
}
;

exp_list:
exp_list COMMA exp {
	$$ = $1;
	$$->code.insert($$->code.begin(), $3->code.begin(), $3->code.end());
	delete $3;
	}
| exp {
	$$ = new statement();
	$$->code = $1->code;
	delete $1;
	}
;

assignment:
IDENTIFIER OP_ASSIGNMENT exp {
	if(symtab.count(*$1) > 0)  { //does the variable exist?
		if(symtab[*$1].typ == $3->typ ) { //type matches?
			symtab[*$1].writ = d_loc__.first_line; // mark as written into

			$$ = new statement();
			$$->code = $3->code;
			$$->code.push_back( codeline(STORE_X, symtab[*$1].num) );
		}
		else {
			error("Type mismatch (both sides of the assignment must have the same type)");
		}
	}
	else {
		std::stringstream ss;
		ss << "Variable '" << *$1 << "' undeclared." << std::endl;
		error(ss.str().c_str());
	}

	delete $1;
	delete $3;
}
;

exp:
IDENTIFIER {
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
			$$->code.push_back( codeline(FETCH_X, symtab[*$1].num) );
		}
		else {
			std::stringstream ss;
			ss << "Variable '" << *$1 << "' undeclared." << std::endl;
			error(ss.str().c_str());
		}
		delete $1;
	}
| T_OPEN exp T_CLOSE {
	$$ = $2;
	}
| constant {
	$$ = $1;
	}
| exp OP_PLUS exp {
		//both operands must be integers
		if($1->typ == $3->typ && $1->typ == integer) {
			$$ = new expression($1->typ);

			$$->code.insert( $$->code.begin(), $1->code.begin(), $1->code.end() );	// első operandus
			$$->code.insert( $$->code.end(), $3->code.begin(), $3->code.end() );	// második operandus

			$$->code.push_back( codeline(ADDI, 0) );

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

		//both operands must be integers
		if($1->typ == $3->typ && $1->typ == integer) {
			$$ = new expression($1->typ);

			$$->code.insert( $$->code.begin(), $1->code.begin(), $1->code.end() );	// első operandus
			$$->code.insert( $$->code.end(), $3->code.begin(), $3->code.end() );	// második operandus

			$$->code.push_back( codeline(SUBI, 0) );

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

		//both operands must be integers
		if($1->typ == $3->typ && $1->typ == integer) {
			$$ = new expression($1->typ);

			$$->code.insert( $$->code.begin(), $1->code.begin(), $1->code.end() );	// első operandus
			$$->code.insert( $$->code.end(), $3->code.begin(), $3->code.end() );	// második operandus

			$$->code.push_back( codeline(DIVI, 0) );

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

		//both operands must be integers
		if($1->typ == $3->typ && $1->typ == integer) {

			$$ = new expression($1->typ);

			$$->code.insert( $$->code.begin(), $1->code.begin(), $1->code.end() );	// első operandus
			$$->code.insert( $$->code.end(), $3->code.begin(), $3->code.end() );	// második operandus

			$$->code.push_back( codeline(MODI, 0) );

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

		//both operands must be integers
		if($1->typ == $3->typ && $1->typ == integer) {
			$$ = new expression($1->typ);

			$$->code.insert( $$->code.begin(), $1->code.begin(), $1->code.end() );	// első operandus
			$$->code.insert( $$->code.end(), $3->code.begin(), $3->code.end() );	// második operandus

			$$->code.push_back( codeline(MULI, 0) );

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

		//both operands must be booleans
		if($1->typ == $3->typ && $1->typ == boolean) {

			$$ = new expression($1->typ);

			$$->code.insert( $$->code.begin(), $1->code.begin(), $1->code.end() );	// első operandus
			$$->code.insert( $$->code.end(), $3->code.begin(), $3->code.end() );	// második operandus

			$$->code.push_back( codeline(AND, 0) );

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

		//both operands must be booleans
		if($1->typ == $3->typ && $1->typ == boolean) {

			$$ = new expression($1->typ);

			$$->code.insert( $$->code.begin(), $1->code.begin(), $1->code.end() );	// első operandus
			$$->code.insert( $$->code.end(), $3->code.begin(), $3->code.end() );	// második operandus

			$$->code.push_back( codeline(OR, 0) );

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
	$$ = $2;
	$$->code.push_back( codeline(NOT, 0) );
	}
| exp OP_EQUALITY exp {

		//operands must be of the same type
		if($1->typ == $3->typ) {

			$$ = new expression(boolean);

			$$->code.insert( $$->code.begin(), $1->code.begin(), $1->code.end() );	// első operandus
			$$->code.insert( $$->code.end(), $3->code.begin(), $3->code.end() );	// második operandus

			$$->code.push_back( codeline(EQ, 0) );

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

		//both operands must be integers
		if($1->typ == $3->typ && $1->typ == integer) {
			$$ = new expression(boolean);

			$$->code.insert( $$->code.begin(), $1->code.begin(), $1->code.end() );	// első operandus
			$$->code.insert( $$->code.end(), $3->code.begin(), $3->code.end() );	// második operandus

			$$->code.push_back( codeline(LESS, 0) );

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

		//both operands must be integers
		if($1->typ == $3->typ && $1->typ == integer) {
			$$ = new expression(boolean);

			$$->code.insert( $$->code.begin(), $1->code.begin(), $1->code.end() );	// első operandus
			$$->code.insert( $$->code.end(), $3->code.begin(), $3->code.end() );	// második operandus

			$$->code.push_back( codeline(GREATER, 0) );

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
	$$->code.push_back( codeline(PSHB, 1) );
	}
| L_FALSE {
	$$ = new expression(boolean);
	$$->code.push_back( codeline(PSHB, 0) );
	}
| L_INTEGER {
	$$ = new expression(integer);
	$$->code.push_back( codeline(PUSH, get_value(*$1)) );
	delete $1;
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
