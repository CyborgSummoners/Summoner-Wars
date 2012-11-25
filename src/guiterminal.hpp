#ifndef TERMINAL_HPP
#define TERMINAL_HPP

#include "SFML/Window.hpp"
#include "SFML/Graphics.hpp"
#include <vector>

namespace sum {

class Game;

class GuiTerminal {
	public:
		// Bemenetként kap egy stringet, ez a sor, amit a user beírt. Feltesszük, hogy nem üres.
		// ezt a stringet feldolgozzuk, végrehajtjuk a parancsot,
		// és visszaadjuk a választ, amit kiírunk a képernyőre.
		// A válasz sikeres végrehajtás esetén üres string.
		std::string command(const std::string& input);

		// Visszaadja az aktuális patht, pl "/bin" vagy "/spells/buffs" vagy "/"
		std::string get_working_directory();
		
		GuiTerminal(sf::RenderWindow *_window);
		
		void Draw();
		void handleEvent(sf::Event &event);
	private:
		
		class Buffer
		{
		
		private:
			std::vector<std::string> strbuffer;
			int act;
			int size;
			int head;
			std::string val;
		public:
			Buffer(int _size=200);
			void up();
			void down();
			void enter();
		};

		float x,y;
		int width;
		int height;
		sf::Color bgColor;
		std::string fosom;
		sf::String input;
		sf::RenderWindow *window;
		Buffer buffer;
};


}

#endif
