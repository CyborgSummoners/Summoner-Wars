#include "guiterminal.hpp"
#include <iostream>

namespace sum{

GuiTerminal::GuiTerminal(sf::RenderWindow *_window) :
Widget(_window),
inputfield_size(25)
{
	window=_window;
	width=window->GetWidth();
	height=window->GetHeight()/3;
	x=0;
	y=window->GetHeight()-height;
	inputfield=new InputField(_window,5,_window->GetHeight()-inputfield_size);
	textbox=new TextBox(_window,5,y,width,height-(inputfield_size*2));
	term=new Terminal();
}

GuiTerminal::~GuiTerminal()
{
	delete inputfield;
	delete textbox;
	delete term;
}

void GuiTerminal::draw()
{
	window->Draw(sf::Shape::Rectangle(
		0,
		window->GetHeight()-height ,
		width,
		window->GetHeight()-height + 2 ,
		textColor));
	inputfield->draw();
	textbox->draw();
}

void GuiTerminal::handleEvent(sf::Event &event)
{
	if((event.Type == sf::Event::KeyPressed) && (event.Key.Code == sf::Key::Return))
	{
		textbox->add(inputfield->val());
		std::vector<std::string> ret = explode(term->command(inputfield->val()), '\n');
		for(int i=0; i<ret.size(); ++i)
			textbox->add(ret[i]);
		inputfield->set("");
	}
	else
	{
		inputfield->handleEvent(event);
	}
}

std::vector<std::string> GuiTerminal::explode(const std::string& str, const char& ch) {
    std::string next = "";
    std::vector<std::string> result;

    for (std::string::const_iterator it = str.begin(); it != str.end(); it++) {
    	if (*it == ch) {
    		if (next.length() > 0) {
    			result.push_back(next);
    			next = "";
    		}
    	} else {
    		next += *it;
    	}
    }

    return result;
}


}
