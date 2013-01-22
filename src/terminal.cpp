#include "terminal.hpp"
#include "game.hpp"
#include "util/debug.hpp"
#include <sstream>

namespace sum {
	namespace {
		std::string whitespace = " \t\n\r";
	}

	namespace filesystem {

		struct Executable : public File {
			std::string server_handle;
			Executable(const std::string& fname, const std::string& handle) : File(fname), server_handle(handle) {
			}
			virtual std::string execute(const std::string& args) {
				Game::SendRequest(server_handle, args);
				return Terminal::freezing_return;
			}
		};

	}

	Terminal::Terminal() {
		// building fake filesystem:
		using filesystem::Dir;
		using filesystem::File;

		this->root = new Dir("");
		Dir* bin;
		bin = new Dir("bin");
		root->subdirs.insert(bin);
		bin->files.insert(new filesystem::Executable("shout", "shout"));
		bin->files.insert(new filesystem::Executable("serverdate", "date"));

		Dir* dir;
		dir = new Dir("scripts");
		root->subdirs.insert(dir);
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
		using filesystem::File;
		using filesystem::Path;

		if("pwd" == command) {
			return get_working_directory()+"\n";
		}
		else if("ls" == command) {
			Path path;
			if(!args.empty()) {
				path = string_to_path(args);
				if(path.empty()) return args+": no such directory\n";
			} else path = working_directory;

			std::string Result;
			Dir* wd = path.back();

			if(wd->subdirs.size() > 0) Result.append("Directories:\n");
			for(std::set<Dir*>::const_iterator it=wd->subdirs.begin(); it!=wd->subdirs.end(); ++it) Result.append( (*it)->name + "\n" );

			if(wd->files.size() > 0) Result.append("Files:\n");
			for(std::set<File*>::const_iterator it=wd->files.begin(); it!=wd->files.end(); ++it) Result.append( (*it)->name + "\n" );
			return Result;
		}
		else if("cd" == command) {
			Path path = string_to_path(args);
			if(path.empty()) return args+": no such directory\n";
			working_directory = path;
			return "";
		}
		else { // We're trying to execute a program.
			Path path;
			File* file = 0;

			// the actual program is the bit after the last /, if there's one
			std::string prog_name;
			std::string pathstr;

			size_t found = command.find_last_of('/');
			if(std::string::npos != found) {
				prog_name = command.substr(found+1);
				pathstr = command.substr(0,found);
			} else {
				prog_name = command;
				pathstr = "";
			}

			if(command[0] == '/') { // is it an absolute path?
				path = string_to_path(pathstr);
				if(!path.empty()) file = get_file(path, prog_name);

			}
			else if(command[0] == '.') { // or relative to here?
				path = string_to_path(get_working_directory() + pathstr);
				if(!path.empty()) file = get_file(path, prog_name);
			}
			else {
				// maybe it's relative anyway?
				path = string_to_path(get_working_directory() + pathstr);
				if(!path.empty()) file = get_file(path, prog_name);

				if(!file) {
					//then maybe it's relative to bin?
					path = string_to_path("/bin/"+pathstr);
					if(!path.empty()) file = get_file(path, prog_name);
				}
			}

			if(file) {
				return file->execute(args) + "\n";
			}
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

	filesystem::Path Terminal::string_to_path(std::string str) {
		using filesystem::Path;
		using filesystem::Dir;
		Path path;
		str.substr(0, str.find_first_of(whitespace)).swap(str);
		if('/' == str[0]) {
			path.push_back(root);
			str.substr(1).swap(str);
		}
		else path = working_directory;

		std::stringstream ss(str);
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
				if(!found) return Path();
			}
		}

		return path;
	}


	filesystem::File* Terminal::get_file(filesystem::Path& path, std::string fname) {
		if(path.empty()) return 0;
		for(std::set<filesystem::File*>::const_iterator it=path.back()->files.begin(); it!=path.back()->files.end(); ++it) {
			if( (*it)->name == fname ) {
				return *it;
			}
		}
		return 0;
	}

	const std::string Terminal::freezing_return = "\\";
}
