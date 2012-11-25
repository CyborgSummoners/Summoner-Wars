#ifndef TERMINAL_HPP
#define TERMINAL_HPP

#include "inputfield.hpp"

namespace sum {

class Game;

class GuiTerminal : public Widget {
	public:
		
		GuiTerminal(sf::RenderWindow *_window);
		~GuiTerminal();
		
		void draw();
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
		int width;
		int height;
		Buffer buffer;
		InputField *inputfield;
		const int inputfield_size;
		//TextBox textbox;
};


}

#endif
