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
			Executable(const std::string& fname) : File(fname) {}
			virtual std::string execute(const std::string& args, sum::Terminal* context = 0) = 0;
			virtual std::string read() const {
				return "Fatal: not a readable file.";
			}
		};

		struct Server_executable : public Executable {
			std::string server_handle;
			Server_executable(const std::string& fname, const std::string& handle) : Executable(fname), server_handle(handle) {
			}
			virtual std::string execute(const std::string& args, sum::Terminal* context = 0) {
				Game::SendRequest(server_handle, args);
				return Terminal::freezing_return;
			}
		};

		// catlike. Args is supposed to be a space-delimited list of filenames.
		struct Print : public Executable {
			Print(const std::string& fname) : Executable(fname) {}

			virtual std::string execute(const std::string& args, sum::Terminal* context = 0) {
				if(0 == context) return "Fatal: could not access filesystem.";
				std::vector<std::string> fnames = stringutils::string_explode(stringutils::trim(args), whitespace);

				std::string Result = "";
				File* f;
				for(size_t i=0; i<fnames.size(); ++i) {
					f = context->get_file(fnames[i]);
					if(f) Result.append(f->content);
					else Result.append(fnames[i] + ": no such file.");
				}
				return Result;
			}
		};

		struct Echo : public Executable {
			Echo() : Executable("echo") {}
			virtual std::string execute(const std::string& args, sum::Terminal* context = 0) {
				return args;
			}
		};

	}

	Terminal::Terminal() : root(new filesystem::Dir("")), bin(new filesystem::Dir("bin")) {
		// building fake filesystem:
		using filesystem::Dir;
		using filesystem::File;

		root->subdirs.insert(bin);
		bin->files.insert( new filesystem::Print("cat") );
		bin->files.insert( new filesystem::Echo() );

		Dir* dir;
		dir = new Dir("scripts");
		root->subdirs.insert(dir);

		dir = new Dir("mon");
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
				return file->execute(args, this) + "\n";
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

	bool Terminal::add_server_exe(std::string path, std::string fname, std::string handle) {
		debugf("Registering serverfun %s named %s in %s...", handle.c_str(), fname.c_str(), path.c_str() );
		filesystem::Path p = string_to_path(path);
		if(p.empty()) {
			debugf("failed: no such path\n");
			return false;
		}

		if((p.back()->files.insert(new filesystem::Server_executable(fname, handle))).second) {
			debugf("done.\n");
			return true;
		}
		debugf("failed: could not insert.\n");
		return false;
	}

	bool Terminal::add_readable(std::string path, std::string fname, std::string content) {
		debugf("Adding catable file %s in %s...", fname.c_str(), path.c_str() );
		filesystem::Path p = string_to_path(path);
		if(p.empty()) {
			debugf("failed: no such path\n");
			return false;
		}
		if((p.back()->files.insert(new filesystem::File(fname, content))).second) {
			debugf("done.\n");
			return true;
		}
		debugf("failed: could not insert.\n");
		return false;
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


	filesystem::File* Terminal::get_file(const filesystem::Path& path, std::string fname) {
		if(path.empty()) return 0;
		for(std::set<filesystem::File*>::const_iterator it=path.back()->files.begin(); it!=path.back()->files.end(); ++it) {
			if( (*it)->name == fname ) {
				return *it;
			}
		}
		return 0;
	}

	filesystem::File* Terminal::get_file(std::string path) {
		if(path.empty()) return 0;
		std::string fname = "";
		size_t found = path.find_last_of('/');
		if(std::string::npos != found) {
			fname = path.substr(found+1);
			path = path.substr(0,found);
		}
		else return get_file(working_directory, path);

		if(path[0] == '/') return get_file(string_to_path(path), fname);	//absolute?

		return get_file( string_to_path( path+"/"+get_working_directory() ), fname );
	}

	const std::string Terminal::freezing_return = "\\";
}
