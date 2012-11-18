#ifndef TERMINAL_HPP
#define TERMINAL_HPP 1
#include <string>

namespace sum {

class Terminal {
	public:
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
