#ifndef TERMINAL_HPP
#define TERMINAL_HPP 1
#include <string>
#include <set>
#include <list>

namespace sum {

namespace filesystem {
	struct File {
		const std::string name;
		
		File(const std::string& name) : name(name) {}
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
	private:
		filesystem::Dir* root;
		filesystem::Path working_directory;

	public:
		Terminal();
	
		// Bemenetként kap egy stringet, ez a sor, amit a user beírt.
		// ezt a stringet feldolgozzuk, végrehajtjuk a parancsot,
		// és visszaadjuk a választ, amit kiírunk a képernyőre.
		// A válasz sikeres végrehajtás esetén üres string.
		std::string command(std::string input);

		// Visszaadja az aktuális patht, pl "/bin" vagy "/spells/buffs" vagy "/"
		std::string get_working_directory();
	};
}

#endif
