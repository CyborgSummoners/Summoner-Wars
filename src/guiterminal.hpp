#ifndef TERMINAL_HPP
#define TERMINAL_HPP

#include "inputfield.hpp"

namespace sum {

class Game;

class GuiTerminal : public Widget {
	public:
		
		GuiTerminal(sf::RenderWindow *_window);
		
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

		float x,y;
		int width;
		int height;
		sf::Color bgColor;
		std::string fosom;
		sf::String input;
		Buffer buffer;
		InputField inputfield;
		
};


}

#endif
