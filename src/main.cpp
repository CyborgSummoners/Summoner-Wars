#ifdef INTERPRETER_DEMO
#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include <sstream>
#include "compiler/summparse.h"
#include "interpreter.hpp"

int main(int argc, char** argv) {
	Parser::action act = Parser::FULL;

	bool error=false;
	std::string src_fname;
	bool got_input = false;
	std::stringstream error_msg;
	for(int i=1; i<argc; ++i) {
		if((strcmp(argv[i], "-a") == 0) || (strcmp(argv[i], "--assembled") == 0)) {
			act=Parser::ASSEMBLED;
		}
		else if((strcmp(argv[i], "-b")==0) || (strcmp(argv[i], "--bytecode") == 0)) {
			act=Parser::BYTECODE;
		}
		else if(argv[i][0]=='-') {
			error=true;
			error_msg << "unknown option: " << argv[i] << ". Valid options are: -a (human-readable assembled) or -b (bytecode). And they can't be combined." << std::endl;
			act=Parser::MALFORMED_ARGS;
		}
		else if(got_input) {
			error_msg << "too many files: I can only process one source file at a time" << std::endl;
			act=Parser::MALFORMED_ARGS;
			error=true;
		}
		else {
			src_fname=std::string(argv[i]);
			got_input=true;
		}
	}

	if(!got_input) act=Parser::NOINPUT;
	else if(act == Parser::MALFORMED_ARGS) {
		std::cerr << error_msg.str() << std::endl;
		error=true;
	}
	else if(act == Parser::NOINPUT) {
		std::cerr << "No input files." << std::endl << "Usage: " << argv[0] << " [optionally -a OR -b] file" << std::endl;
		error=true;
	}

	if(error) return error;

	std::ifstream source(src_fname.c_str());
	if(!source.is_open() || !source.good()) {
		error=true;
		std::cerr << "Could not open source file '" << src_fname << "' for reading." << std::endl;
		return 1;
	}

	if(error) return error;

	Parser parser(source, std::cout, act);
	error=parser.parse();
	source.close();

	if(error) return error;
	if(act != Parser::FULL) return 0;

	sum::Interpreter interpreter;

	for(size_t i=0; i<parser.subprograms.size(); ++i) {
		interpreter.register_subprogram(parser.subprograms[i]);
	}

	interpreter.execute(parser.subprograms[0].get_name());

	return error;
}
#else

#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include "game.hpp"
#include "interpreter.hpp"
#include "puppet.hpp"
#include "compiler/summparse.h"
#include <fstream>
#include <iostream>

int main()
{
	std::ifstream source("script-samples/puppet_demo.summ");
	if(!source.is_open() || !source.good()) {
		std::cerr << "Could not open source file 'script-samples/puppet_demo.summ' for reading." << std::endl;
		return 1;
	}
	Parser parser(source, std::cout);
	parser.parse();
	sum::Interpreter interpreter;
	for(size_t i=0; i<parser.subprograms.size(); ++i) {
		interpreter.register_subprogram(parser.subprograms[i]);
	}
	// ^ ezeket itt marha jó lenne betenni... akár az Interpreter konstruktorába? tagfüggvényébe? (load_file és load_dir vagy ilyesmi)

	sum::Puppet puppet("Blue Square");
	interpreter.register_puppet(puppet);
	interpreter.set_behaviour(puppet, "DEMO");

	sum::Puppet p2("Red Circle");
	interpreter.register_puppet(p2);
	interpreter.set_behaviour(p2, "DEMO");

	while(true) {
		interpreter.step(10);
		sf::Sleep(0.2f);
	}

    //sum::Game::Start();
    return EXIT_SUCCESS;
}

#endif
