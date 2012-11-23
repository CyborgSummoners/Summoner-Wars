#ifndef TERMINAL_HPP
#define TERMINAL_HPP

#include "SFML/Window.hpp"
#include "SFML/Graphics.hpp"

namespace sum {

class Game;

class Terminal {
	public:
		// Bemenetként kap egy stringet, ez a sor, amit a user beírt. Feltesszük, hogy nem üres.
		// ezt a stringet feldolgozzuk, végrehajtjuk a parancsot,
		// és visszaadjuk a választ, amit kiírunk a képernyőre.
		// A válasz sikeres végrehajtás esetén üres string.
		std::string command(const std::string& input);

		// Visszaadja az aktuális patht, pl "/bin" vagy "/spells/buffs" vagy "/"
		std::string get_working_directory();
		
		Terminal(sf::RenderWindow &window);
		
		void Draw(sf::RenderWindow &window);
		void handleEvent(sf::Event &event);
	private:
		float x,y;
		int size;
		sf::String input;
};


}

#endif
