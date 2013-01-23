#ifndef TERMINAL_HPP
#define TERMINAL_HPP 1
#include <string>
#include <set>
#include <list>

namespace sum {

namespace filesystem {
	struct File {
		const std::string name;
		const std::string content;

		File(const std::string& name, const std::string& content = "") : name(name), content(content) {}

		virtual std::string execute(const std::string& args) {
			return "Not an executable file.";
		}

		virtual std::string read() const {
			return content;
		}
	};

	struct Dir {
		const std::string name;
		std::set<Dir*> subdirs;
		std::set<File*> files;

		Dir(const std::string& name) : name(name) {}
	};

	typedef std::list<filesystem::Dir*> Path;
}


class Terminal {
	public:
		static const std::string freezing_return;

	private:
		filesystem::Dir* root;
		filesystem::Dir* bin;
		filesystem::Path working_directory;

	private:
		filesystem::Path string_to_path(std::string path);
		filesystem::File* get_file(filesystem::Path& path, std::string fname);

	public:
		Terminal();

		// Bemenetként kap egy stringet, ez a sor, amit a user beírt.
		// ezt a stringet feldolgozzuk, végrehajtjuk a parancsot,
		// és visszaadjuk a választ, amit kiírunk a képernyőre.
		// A válasz sikeres végrehajtás esetén üres string.
		std::string command(std::string input);

		// Visszaadja az aktuális patht, pl "/bin" vagy "/spells/buffs" vagy "/"
		std::string get_working_directory();

		bool add_server_exe(std::string path, std::string fname, std::string handle);
		bool add_readable(std::string path, std::string fname, std::string content);
	};
}

#endif
