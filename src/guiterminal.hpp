#ifndef GUITERMINAL_HPP
#define GUITERMINAL_HPP

#include "inputfield.hpp"
#include "textbox.hpp"
#include "terminal.hpp"

namespace sum {

class Game;

class GuiTerminal : public Widget {
	public:
		
		GuiTerminal(sf::RenderWindow *_window);
		~GuiTerminal();
		
		void draw();
		void handleEvent(sf::Event &event);
		
	private:

		int width;
		int height;
		InputField *inputfield;
		TextBox *textbox;
		Terminal *term;
		const int inputfield_size;
		
		static std::vector<std::string> explode(const std::string& str, const char& ch);
		
};


}

#endif
