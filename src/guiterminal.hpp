#ifndef GUITERMINAL_HPP
#define GUITERMINAL_HPP

#include "inputfield.hpp"
#include "textbox.hpp"
#include "terminal.hpp"

namespace sum {

class GuiTerminal : public Widget, public Observer<ServerMessage>
{
	
	public:
		
		GuiTerminal(
			sf::RenderWindow *_window,
			std::string _player_name,
			int _width = 0,
			int _height = 0);
		~GuiTerminal();
		
		void draw();
		void handleEvent(sf::Event &event);
		void update(const ServerMessage &message);
		
		
	private:
	
		struct Buffer
		{
		
			std::vector<std::string> buff;
			int size;
			std::vector<std::string>::iterator act;
			
			Buffer(int _size=100) : size(_size),act(0){}
			
			bool up();
			std::string val();
			bool is_end();
			bool down();
			void enter(std::string _val);
		};

		std::string player_name;
		sf::String name_pwd;
		InputField *inputfield;
		TextBox *textbox;
		Terminal *term;
		Buffer buffer;
		const int inputfield_size;
		bool frozen;
		
};


}

#endif
