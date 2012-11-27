#ifndef TERMINAL_HPP
#define TERMINAL_HPP

#include "inputfield.hpp"
#include "textbox.hpp"

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
		const int inputfield_size;
};


}

#endif
