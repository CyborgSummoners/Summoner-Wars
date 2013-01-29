#include "terminal.hpp"
#include "game.hpp"
#include "filehandling.hpp"
#include "util/debug.hpp"
#include <sstream>
#include <fstream>
#include <cassert>
#include <algorithm>
#include <stdexcept>

// dangerously tight coupling:
#include "parser.hpp"
#include "bytecode.hpp"
#include "compiler/summparse.h"
#include <SFML/Network.hpp>


namespace sum {
	namespace {
		std::string whitespace = " \t\n\r";
	}

	namespace filesystem {
		struct Mounted_file : public File {
			const std::string realpath;
			Mounted_file(const std::string& name, const std::string& realpath) : File(name), realpath(realpath) {}

			virtual std::string read() const {
				std::stringstream ss;
				filehandling::read(realpath, ss);
				return ss.str();
			}
		};

		struct Mounted_dir : public Dir {
			const std::string mounted_dir;
			Mounted_dir(const std::string& name, const std::string& dir_to_mount) : Dir(name), mounted_dir(dir_to_mount) {
				refresh();
			}

			virtual void refresh() {
				// naively drop/insert. This will cause Bad Things if a dir is mounted in a mounted dir -- but shit, don't.
				// also, it's recursive.
				//debugf("Refreshing %s\n", mounted_dir.c_str());
				for(std::set<File*>::iterator it=files.begin(); it!=files.end(); ++it) delete *it;
				for(std::set<Dir*>::iterator it=subdirs.begin(); it!=subdirs.end(); ++it) delete *it;
				subdirs.clear();
				files.clear();

				//get dirs
				std::set<std::string> names = filehandling::get_subdirs(mounted_dir);
				for(std::set<std::string>::const_iterator it = names.begin(); it!=names.end(); ++it) {
					subdirs.insert(new Mounted_dir(*it, mounted_dir+"/"+*it));
				}
				names = filehandling::get_files(mounted_dir);
				for(std::set<std::string>::const_iterator it = names.begin(); it!=names.end(); ++it) {
					files.insert(new Mounted_file(*it, mounted_dir+"/"+*it));
				}

			}
		};

		struct Executable : public File {
			Executable(const std::string& fname, const Terminal::Completer& completer = Terminal::dir_completer) : File(fname, "", completer) {}
			virtual std::string execute(const std::string& args, sum::Terminal* context = 0) = 0;
			virtual bool is_readable() const {
				return false;
			}
			virtual std::string read() const {
				throw std::logic_error("Fatal: not a readable file.");
				return "Fatal: not a readable file.";
			}
			virtual bool is_executable() const{
				return true;
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
			Print(const std::string& fname) : Executable(fname, Terminal::filedir_completer) {}

			virtual std::string execute(const std::string& args, sum::Terminal* context = 0) {
				if(0 == context) return "Fatal: could not access filesystem.";
				if(stringutils::trim(args).empty()) return "";

				std::vector<std::string> fnames = stringutils::string_explode(stringutils::trim(args), whitespace);

				std::string Result = "";
				File* f;
				for(size_t i=0; i<fnames.size(); ++i) {
					f = context->get_file(fnames[i]);
					if(f) {
						if(f->is_readable()) Result.append(f->read());
						else Result.append("Fatal: not a readable file.");
					}
					else Result.append(fnames[i] + ": no such file.");
				}
				return Result;
			}

		};

		struct Echo : public Executable {
			Echo() : Executable("echo", Terminal::filedir_completer) {}
			virtual std::string execute(const std::string& args, sum::Terminal* context = 0) {
				return args;
			}
		};

		struct Pwd : public Executable {
			Pwd() : Executable("pwd") {}
			virtual std::string execute(const std::string& args, sum::Terminal* context) {
				if(0 == context) return "Fatal: could not access filesystem.";
				return context->get_working_directory();
			}
			virtual void complete(const std::string& fragment, std::set<std::string>& Result, sum::Terminal* context) const {}
		};
		struct Cd : public Executable {
			Cd() : Executable("cd", Terminal::dir_completer) {}
			virtual std::string execute(const std::string& args, sum::Terminal* context) {
				if(0 == context) return "Fatal: could not access filesystem.";
				Path path = context->string_to_path(args);
				if(path.empty()) return args+": no such directory";
				context->working_directory = path;
				return "";
			}
		};
		struct Mount : public Executable {
			Mount() : Executable("mount") {}
			virtual std::string execute(const std::string& args, sum::Terminal* context) {
				assert(context);
				if(stringutils::trim(args).empty()) return "Not enough argument to function.\nUsage: mount <physical-subdir-name> <dir-name>";
				std::vector<std::string> argv = stringutils::string_explode(stringutils::trim(args), whitespace);
				if(argv.size() < 2) return "Not enough argument to function.\nUsage: mount <physical-subdir-name> <new-dir-name> ";
				if(argv.size() > 2) return "Too many arguments to function.\nUsage: mount <physical-subdir-name> <new-dir-name> ";

				if(std::count_if(argv[1].begin(), argv[1].end(), stringutils::is_valid_path_char) != (signed)argv[1].size() ) return argv[1] + " is not a valid path name.";

				// local dir exists already?
				Path path = context->string_to_path(argv[1]);
				if(!path.empty()) return argv[1] + " already exists.";
				if(!filehandling::dir_exists(argv[0])) {
					return "Outer directory "+argv[0]+" does not seem to exist.";
				}

				//make dir. this is awkward and terrible, but dunno now. FIXME.
				path = context->string_to_path(argv[1], true);
				Dir* dir = path.back();
				std::string dirname = dir->name;

				path.pop_back();
				Dir* parent = path.back();
				parent->subdirs.erase(dir);
				delete dir;
				dir = new Mounted_dir(dirname, argv[0]);
				parent->subdirs.insert(dir);
				path.push_back(dir);

				return "";
			}
		};
		struct Ls : public Executable {
			Ls() : Executable("ls", Terminal::dir_completer) {}
			virtual std::string execute(const std::string& args, sum::Terminal* context) {
				if(0 == context) return "Fatal: could not access filesystem.";
				Path path;
				if(!args.empty()) {
					path = context->string_to_path(args);
					if(path.empty()) return args+": no such directory\n";
				} else path = context->working_directory;

				std::string Result;
				Dir* wd = path.back();

				if(wd->subdirs.size() > 0) Result.append("Directories:\n");
				for(std::set<Dir*>::const_iterator it=wd->subdirs.begin(); it!=wd->subdirs.end(); ++it) Result.append( (*it)->name + "\n" );

				if(wd->files.size() > 0) Result.append("Files:\n");
				for(std::set<File*>::const_iterator it=wd->files.begin(); it!=wd->files.end(); ++it) Result.append( (*it)->name + "\n" );
				return Result;
			}
		};

		// dangerously tight coupling.
		struct Compile : public Executable {
			Compile() : Executable("compile", Terminal::filedir_completer) {}
			virtual std::string execute(const std::string& args, sum::Terminal* context) {
				assert(context);
				if(stringutils::trim(args).empty()) return "Not enough argument to function.\nUsage: compile <path>*";
				std::vector<std::string> argv = stringutils::string_explode(stringutils::trim(args), whitespace);
				std::stringstream Result;

				File* f;
				Path path;
				std::vector<File*> files;
				for(size_t i=0; i<argv.size(); ++i) {
					f = context->get_file(argv[i]);

					if(f) files.push_back(f);
					else { //maybe it's a path?
						path = context->string_to_path(argv[i]);
						if(!path.empty()) {
							Dir* dir = path.back();
							for(std::set<File*>::const_iterator it=dir->files.begin(); it != dir->files.end(); ++it) {
								files.push_back(*it);
							}
						}
						else {
							Result << argv[i] + ": no such file or directory." << std::endl;
						}
					}
				}

				if(files.empty()) {
					Result << "Error: no input files." << std::endl;
					return Result.str();
				}

				::Parser parser(Result);
				for(size_t i=0; i<files.size(); ++i) {
					try {
						if(files[i]->is_readable()) {
							Result << "compile " << files[i]->name << std::endl;
							std::stringstream in;
							in.str( files[i]->read() );
							parser.parse(in);
						}
						else Result << "Error: could not open " << files[i]->name << " for reading." << std::endl;
					}
					catch(std::exception& e) {
						Result << "Error: " << e.what() << std::endl;
					}
				}

				using sum::Parser::operator<<;
				if(parser.subprograms.size() > 0) {
					sf::Packet packet;
					packet << "scriptreg";
					packet << static_cast<sf::Uint32>(parser.subprograms.size());
					for(size_t i=0; i<parser.subprograms.size(); ++i) {
						packet << parser.subprograms[i];
					}

					Game::SendPacket(packet);
					Result << Terminal::freezing_return;
				}

				return Result.str();
			}
		};

	}

	Terminal::Terminal() : root(new filesystem::Dir("")), bin(new filesystem::Dir("bin")), physical_scripts(new filesystem::Dir("src")) {
		// building fake filesystem:
		using filesystem::Dir;
		using filesystem::File;

		root->subdirs.insert(bin);
		bin->files.insert( new filesystem::Print("cat") );
		bin->files.insert( new filesystem::Echo() );
		bin->files.insert( new filesystem::Pwd() );
		bin->files.insert( new filesystem::Cd() );
		bin->files.insert( new filesystem::Ls() );
		bin->files.insert( new filesystem::Mount() );
		bin->files.insert( new filesystem::Compile() );

		Dir* dir;

		dir = new Dir("mon");
		root->subdirs.insert(dir);

		dir = new Dir("mnt");
		root->subdirs.insert(dir);
//		dir->subdirs.insert(physical_scripts);

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

		{ // We're trying to execute a program.
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


	std::set<std::string> Terminal::complete(const std::string& prefix, std::string input) {
		std::set<std::string> Result;

		// is the command finished?
		if(prefix.empty()) {
			//no, it isn't. we'll be handing out a set of command strings with the prefix $input.
			filesystem::Path path;
			std::string command;
			std::string pathstr;

			// is the directory specified?
			size_t per = input.find_last_of('/');
			if(!input.empty() && per!=std::string::npos) {
				//okay, we'll restrict the search to that dir (and bin)
				command = input.substr(per+1);
				path = string_to_path( input.substr(0,per) );
			}
			else {
				command = input;
				path = working_directory;
				// okay, then we'll give you executables in current dir (and in bin), and
			}

			if(!path.empty()) {
				size_t prefixlen = command.size();

				// all exes in bin:
				for(std::set<filesystem::File*>::const_iterator it=bin->files.begin(); it!=bin->files.end(); ++it) {
					if( (*it)->is_executable() && (*it)->name.substr(0, prefixlen) == command ) Result.insert( (*it)->name + " " );
				}

				// all exes in specified directory:
				// um, currently this is totally meaningless.
				filesystem::Dir* dir = path.back();
				if(dir != bin) {
					for(std::set<filesystem::File*>::const_iterator it=dir->files.begin(); it!=dir->files.end(); ++it) {
						if( (*it)->is_executable() && (*it)->name.substr(0, prefixlen) == command ) Result.insert( (*it)->name + " " );
					}
				}

				// todo: directories.
			}
		}
		else {
			//ah good, then it's the command's responsibility.
			//extract command
			std::string command = stringutils::trim(prefix);
			size_t div = command.find_first_of(whitespace);
			command = command.substr(0, div);

			filesystem::File* fil = get_file(command);
			if(!fil) fil = get_file("/bin/"+command);

			if(fil) {	// if file actually exists
				fil->complete( input, Result, this);
			}
		}

		return Result;
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

	filesystem::Path Terminal::string_to_path(std::string str, bool makeit) {
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
						path.back()->refresh(); // this is great, but should be temporary. ATM we can't be sure that everything around uses this fun.
						found = true;
						break;
					}
				}
				if(!found && makeit) {
					Dir* dir = new Dir(part);
					path.back()->subdirs.insert(dir);
					path.push_back(dir);
				}
				else if(!found) return Path();
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


	void Terminal::pathfname(std::string& command, std::string& path) {
		size_t per = command.find_last_of("/");
		if(per != std::string::npos) {
			path    = command.substr(0,per);
			command = command.substr(per+1);
		}
	}


	// puts every directory whose path is prefixed with fragment into the resultset
	struct Dir_completer : public Terminal::Completer {
		virtual void complete(const std::string& fragment, std::set<std::string>& Result, Terminal* context) const {
			assert(context);
			filesystem::Path path;
			std::string pathstr;
			std::string frag = fragment;

			if(!frag.empty()) Terminal::pathfname(frag, pathstr);

			path = context->string_to_path(pathstr);
			if(path.empty()) return;
			filesystem::Dir* dir = path.back();
			size_t prefixlen = frag.size();
			if(!pathstr.empty()) pathstr.append("/");
			if(prefixlen > 0 && frag[0]=='.') Result.insert(pathstr + "../");
			for(std::set<filesystem::Dir*>::const_iterator it = dir->subdirs.begin(); it!=dir->subdirs.end(); ++it) {
				if( (*it)->name.substr(0, prefixlen) == frag ) Result.insert( pathstr+(*it)->name + "/" );
			}
		}
	};


	struct File_completer : public Terminal::Completer {
		virtual void complete(const std::string& fragment, std::set<std::string>& Result, Terminal* context) const {
			assert(context);
			filesystem::Path path;
			std::string pathstr;
			std::string frag = fragment;

			if(!frag.empty()) Terminal::pathfname(frag, pathstr);

			path = context->string_to_path(pathstr);
			if(path.empty()) return;
			filesystem::Dir* dir = path.back();
			size_t prefixlen = frag.size();
			if(!pathstr.empty()) pathstr.append("/");

			for(std::set<filesystem::File*>::const_iterator it = dir->files.begin(); it!=dir->files.end(); ++it) {
				if( (*it)->name.substr(0, prefixlen) == frag ) Result.insert( pathstr+(*it)->name + " " );
			}
		}
	};

	struct Filedir_completer : public Terminal::Completer {
		virtual void complete(const std::string& fragment, std::set<std::string>& Result, Terminal* context) const {
			Terminal::dir_completer.complete(fragment, Result, context);
			Terminal::file_completer.complete(fragment, Result, context);
		}
	};


	const Terminal::Completer& Terminal::dir_completer = Dir_completer();
	const Terminal::Completer& Terminal::file_completer = File_completer();
	const Terminal::Completer& Terminal::filedir_completer = Filedir_completer();
	const std::string Terminal::freezing_return = "\\";
}
