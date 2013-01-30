#ifndef TERMINAL_HPP
#define TERMINAL_HPP 1
#include <string>
#include <set>
#include <list>

namespace sum {

namespace filesystem {
	struct Dir;
	struct File;
	typedef std::list<filesystem::Dir*> Path;
}

class Terminal {
	public:
		static const std::string freezing_return;

	private:
		filesystem::Dir* root;
		filesystem::Dir* bin;
		filesystem::Dir* physical_scripts;

	public:
		filesystem::Path working_directory;
		filesystem::Path string_to_path(std::string path, bool makeit = false);
		filesystem::File* get_file(const filesystem::Path& path, std::string fname);
		filesystem::File* get_file(std::string path);


		Terminal();

		// Bemenetként kap egy stringet, ez a sor, amit a user beírt.
		// ezt a stringet feldolgozzuk, végrehajtjuk a parancsot,
		// és visszaadjuk a választ, amit kiírunk a képernyőre.
		// A válasz sikeres végrehajtás esetén üres string.
		std::string command(std::string input);

		std::set<std::string> complete(const std::string& prefix, std::string input);

		// Visszaadja az aktuális patht, pl "/bin" vagy "/spells/buffs" vagy "/"
		std::string get_working_directory();

		bool add_server_exe(std::string path, std::string fname, std::string handle);
		bool add_readable(std::string path, std::string fname, std::string content);

		//utility
		static void pathfname(std::string& fname, std::string& path);

	public:
		struct Completer {
			virtual void complete(const std::string& fragment, std::set<std::string>& Result, Terminal* context) const = 0;
			virtual ~Completer() {}
		};

		static const Completer& dir_completer;
		static const Completer& file_completer;
		static const Completer& filedir_completer;
};


namespace filesystem {

	namespace {
		template<typename T>
		struct ptr_less {
			bool operator()(T* const lhs, T* const rhs) const {
				return *(lhs) < *(rhs);
			}
		};
	}

	struct File {
		const std::string name;
		const std::string content;
		const Terminal::Completer& completer;

		File(const std::string& name, const std::string& content = "", const Terminal::Completer& completer = Terminal::dir_completer) : name(name), content(content), completer(completer) {}

		virtual std::string execute(const std::string& args, sum::Terminal* context = 0) {
			return "Not an executable file.";
		}

		virtual std::string read() const {
			return content;
		}

		virtual bool is_readable() const {
			return true;
		}

		virtual bool is_executable() const{
			return false;
		}

		virtual void complete(const std::string& fragment, std::set<std::string>& Result, sum::Terminal* context) const {
			this->completer.complete(fragment, Result, context);
		}

		bool operator<(const File& that) const {
			return this->name < that.name;
		}
	};

	struct Dir {
		const std::string name;
		std::set<Dir*, ptr_less<Dir> > subdirs;
		std::set<File*, ptr_less<File> > files;

		Dir(const std::string& name) : name(name) {}
		virtual void refresh() {}
		virtual ~Dir() {
			for(std::set<Dir*>::iterator it=subdirs.begin(); it!=subdirs.end(); ++it) delete *it;
			for(std::set<File*>::iterator it=files.begin(); it!=files.end(); ++it) delete *it;
		}
		bool operator<(const Dir& that) const {
			return this->name < that.name;
		}
	};

}
}

#endif
