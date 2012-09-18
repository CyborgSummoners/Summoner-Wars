%lsp-needed
%baseclass-preinclude <iostream>

%token IDENTIFIER
%token K_PROCEDURE
%token K_IS
%token K_BEGIN
%token K_END
%token K_NULL

%%

start:
procedure {
	std::cout << "start -> procedure" << std::endl;
	}
;

procedure:
signature proc_body {
	std::cout << "procedure -> signature proc_body" << std::endl;
	}
;

signature:
K_PROCEDURE IDENTIFIER K_IS {
	std::cout << "signature -> K_PROCEDURE IDENTIFIER K_IS" << std::endl;
	}
;

proc_body:
K_BEGIN K_NULL K_END {
	std::cout << "proc_body -> K_BEGIN K_NULL K_END" << std::endl;
	}
|
K_BEGIN K_NULL K_END IDENTIFIER {
	std::cout << "proc_body -> K_BEGIN K_NULL K_END IDENTIFIER" << std::endl;
	}
;