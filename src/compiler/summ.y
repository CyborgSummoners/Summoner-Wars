%lsp-needed
%baseclass-preinclude "compiler.hpp"

%token LEXICAL_ERROR

%type <exp> constant
%type <exp> exp

%type <stmt> assignment
%type <stmt> conditional
%type <stmt> conditional_branches
%type <stmt> loop
%type <stmt> proc_call
%type <stmt> statement
%type <stmt> statements
%type <stmt> proc_body
%type <stmt> exp_list
%type <stmt> call_arguments

%type <count> argument_list arguments
%token <str> IDENTIFIER
%token K_PROCEDURE
%token K_IS
%token K_END
%token K_IF
%token K_THEN
%token K_ELSE
%token K_ELSIF
%token K_WHILE
%token K_LOOP

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
%left OP_UNARY_MINUS

%token<str> L_INTEGER
%token<str> L_STRING

%token K_NULL

%union {
	size_t count;
	std::string* str;
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
procedures procedure | procedure;

procedure:
K_PROCEDURE IDENTIFIER argument_list K_IS proc_body K_END IDENTIFIER SEMICOLON {
	if(*$2 != *$7) {
		std::stringstream ss;
		ss << "Name mismatch. Procedure declared as '" << subprogram::normalize_name(*$2) << "' ends as '" << subprogram::normalize_name(*$7) << "'";
		error(ss.str().c_str());
	}
	else {

		//$5->code.insert( $5->code.end(), $6->code.begin(), $6->code.end() );
		$5->code.push_back( codeline(RET, 0) );

		second_pass($5->code);

		byte* code = 0;
		size_t length = 0;
		assemble($5->code, code, length);
		subprograms.push_back( subprogram(*$2, $3, code, length) );
		reset();
	}

	delete $2;
	//delete $5;
	delete $5;
	delete $7;
};

argument_list:
/*epszilon*/
{
	$$ = 0;
}
| T_OPEN arguments T_CLOSE {
	$$ = $2;
};

arguments:
argument {
	$$ = 1;
}
| arguments COMMA argument {
	$$ = $1 + 1;
};

argument:
IDENTIFIER {
	if(symtab.count(*$1) > 0) { // a var with this name exist already?
		std::stringstream ss;
		ss << "Variable '" << *$1 << "' already declared (on line " << symtab.find(*$1)->second.decl << ")." << std::endl;
		error(ss.str().c_str());
	}
	else {
		symtab.insert( make_pair(*$1, var( gen_varnum(), d_loc__.first_line, any)) );
	}

	delete $1;
};


proc_body:
statements {
	$$ = $1;
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

	if( !($2->is(boolean)) ) {
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
K_IF exp K_THEN statements conditional_branches K_END K_IF {
	$$ = new statement();

	if( !($2->is(boolean)) ) {
		error("The condition must be a boolean expression");
	}
	else {
		uint32_t else_label = gen_label();
		uint32_t end_label = gen_label();

		$$->code.insert( $$->code.begin(), $2->code.begin(), $2->code.end() ); //feltétel

		// are there other branches at all?
		if($5->code.size()>0) {
			// replace placeholder JMP 0-s with JMP else_label
			for(code_iterator it=$5->code.begin(); it!=$5->code.end(); ++it) {
				if(it->opcode == JMP && it->argument==0) it->argument=end_label;
			}

			$$->code.push_back( codeline(JMPFALSE, else_label) );	               // if condition is NOT true, we jump to the next branch.
			$$->code.insert( $$->code.end(), $4->code.begin(), $4->code.end() );   // true branch
			$$->code.push_back( codeline(JMP, end_label) );	                       //when finished with the branch, we jump to the end.

			$$->code.push_back( codeline(NOP, 0, else_label) );                    // other branches start here
			$$->code.insert( $$->code.end(), $5->code.begin(), $5->code.end() );

			$$->code.push_back( codeline(NOP, 0, end_label) );                     // end
		}
		else { // otherwise it's just
			$$->code.push_back( codeline(JMPFALSE, end_label) );	               // if condition is NOT true, we jump to the end.
			$$->code.insert( $$->code.end(), $4->code.begin(), $4->code.end() );   // true branch
			$$->code.push_back( codeline(NOP, 0, end_label) );                     // end
		}
	}

	delete $2;
	delete $4;
	delete $5;
};


conditional_branches:
/* epszilon */ {
	$$ = new statement();
}
| conditional_branches K_ELSIF exp K_THEN statements {
	$$ = $1;
	if( !($3->is(boolean)) ) {
		error("The condition must be a boolean expression");
	}
	else {
		uint32_t else_label = gen_label();

		$$->code.insert( $$->code.end(), $3->code.begin(), $3->code.end() ); // feltétel
		$$->code.push_back( codeline(JMPFALSE, else_label) );	               // if condition is NOT true, we jump to the next branch.
		$$->code.insert( $$->code.end(), $5->code.begin(), $5->code.end() );   // this branch
		// Hackery here: 0 is a placeholder which will be replaced when we actually know the end label:
		$$->code.push_back( codeline(JMP, 0) );                                // when finished with the branch, we jump to the end.
		$$->code.push_back( codeline(NOP, 0, else_label) );                    // other branches will start here
	}

	delete $3;
	delete $5;
}
| conditional_branches K_ELSE statements {
	$$ = $1;
	$$->code.insert( $$->code.end(), $3->code.begin(), $3->code.end() );

	delete $3;
};


proc_call:
	IDENTIFIER call_arguments {
		$$ = $2;

		// is it an interrupt?
		std::string norm=subprogram::normalize_name(*$1);
		int intrpt = get_interrupt_id(norm);
		if( intrpt >= 0 ) {
			$$->code.push_back( codeline(INTERRUPT, intrpt) );
		}
		else {
			$$->code.push_back( codeline(CALL, 0, 0, norm ) );
		}

		delete $1;
	}
;

call_arguments:
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
	std::map<std::string, var>::iterator varit = symtab.find(*$1);
	if(varit == symtab.end()) {
		varit = symtab.insert( std::make_pair(*$1, var( gen_varnum(), d_loc__.first_line, $3->typ) ) ).first;
	}
	else symtab.find(*$1)->second.writ = d_loc__.first_line; // mark as written into
	$$ = new statement();
	$$->code = $3->code;
	$$->code.push_back( codeline(RSRV, 1) );
	$$->code.push_back( codeline(STORE_X, symtab.find(*$1)->second.num) );

	delete $1;
	delete $3;
};

exp:
IDENTIFIER {
		$$ = new expression(any);
		if(symtab.count(*$1) > 0) { //does it exist?
			if(symtab.find(*$1)->second.read == 0) { //is it read for the first time?
				symtab.find(*$1)->second.read = d_loc__.first_line;
				if(symtab.find(*$1)->second.read < symtab.find(*$1)->second.writ) { //was it written?
					std::stringstream ss;
					ss << "variable " << *$1 << " doesn't seem to be initialized when first read";
					error(ss.str().c_str());
				}
			}
			$$->typ = symtab.find(*$1)->second.typ;
			$$->code.push_back( codeline(FETCH_X, symtab.find(*$1)->second.num) );
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
| OP_MINUS exp %prec OP_UNARY_MINUS {
	$$ = $2;
	if( $2->is(integer) ) {
		$$->code.push_back( codeline(NEG, 0) );
	} else {
		error("Type mismatch (operand must be integer)\n");
	}
}
| exp OP_PLUS exp {
		//both operands must be integers
		if( $1->is(integer) && $3->is(integer) ) {
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
		if( $1->is(integer) && $3->is(integer) ) {
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
		if( $1->is(integer) && $3->is(integer) ) {
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
		if( $1->is(integer) && $3->is(integer) ) {

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
		if( $1->is(integer) && $3->is(integer) ) {
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
		if( $1->is(boolean) && $3->is(boolean) ) {

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
		if( $1->is(boolean) && $3->is(boolean) ) {

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
	$$->code.push_back( codeline(NEG, 0) );
	}
| exp OP_EQUALITY exp {

		//operands must be of the same type
		if( $1->is($3->typ) ) {

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
		if( $1->is(integer) && $3->is(integer) ) {
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
		if( $1->is(integer) && $3->is(integer) ) {
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
| L_STRING {
	$$ = new expression(string);
	$$->code.push_back( codeline(PSHS, 0, 0, *$1) );
	delete $1;
};
