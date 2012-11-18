#include "terminal.hpp"
#include <sstream>

namespace sum {
	namespace {
		std::string whitespace = " \t\n\r";	
	}
	

	Terminal::Terminal() {		
		// building fake filesystem:
		using filesystem::Dir;
		
		this->root = new Dir("");
		Dir* dir;
		dir = new Dir("bin");
		root->subdirs.insert(dir);
		dir = new Dir("stuff");
		root->subdirs.insert(dir);
		dir->subdirs.insert(new Dir("moar_stuff"));

		// set pwd to root
		this->working_directory.push_back( root );
	}

	std::string Terminal::command(std::string input) {
		// trim left
		size_t s = input.find_first_not_of(whitespace);
		if(std::string::npos != s) input.substr(s).swap(input);

		if(input.empty()) return "";

		// get command -- everything before the first space.
		s = input.find_first_of(whitespace);
		std::string command = input.substr( 0, s );

		std::string args;
		if(std::string::npos != s) args = input.substr(s);
		s = args.find_first_not_of(whitespace);
		if(std::string::npos != s) args.substr(s).swap(args);

		using filesystem::Dir;

		if("pwd" == command) {
			return get_working_directory()+"\n";
		}
		else if("ls" == command) {
			std::string Result;
			Dir* wd = working_directory.back();
			for(std::set<Dir*>::const_iterator it=wd->subdirs.begin(); it!=wd->subdirs.end(); ++it) Result.append( (*it)->name + "\n" );
			return Result;
		}
		else if("cd" == command) {
			using filesystem::Path;
			Path path;
			args.substr(0,args.find_first_of(whitespace)).swap(args);
			if('/' == args[0]) {
				path.push_back(root);
				args.substr(1).swap(args);
			}
			else path = working_directory;
			
			std::stringstream ss(args);
			std::string part;
			while( getline(ss,part,'/') ) {
				if(part.empty() || "." == part) continue;
				if(".." == part) { 
					if(path.size() > 1) path.pop_back();
					else continue;
				}
				else {
					bool found = false;
					for(std::set<Dir*>::const_iterator it=path.back()->subdirs.begin(); it!=path.back()->subdirs.end(); ++it) {
						if( (*it)->name == part ) {
							path.push_back(*it);
							found = true;
							break;
						}
					}
					if(!found) return args+": no such directory\n";
				}
			}
		
			working_directory = path;
			return "";
		}

		return command+": command not found\n";
	}	
	
	std::string Terminal::get_working_directory() {
		if(working_directory.size() < 2) return "/";
		
		std::string Result;
		for(std::list<filesystem::Dir*>::const_iterator it = ++(working_directory.begin()); it!=working_directory.end(); ++it) {
			Result.append("/").append( (*it)->name );
		}
		return Result;
	}
}
