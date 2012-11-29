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
	if((event.Key.Code == sf::Key::Return) && (event.Type == sf::Event::KeyPressed))
	{
		buffer.enter(inputfield->val());
		std::vector<std::string> ret = explode(term->command(inputfield->val()), '\n');
		for(int i=0; i<ret.size(); ++i)
			textbox->add(ret[i]);
		inputfield->reset();
	}
	if((event.Key.Code == sf::Key::Up) && (event.Type == sf::Event::KeyPressed))
	{
		if(buffer.up())
			inputfield->set(buffer.val());
	}
	if((event.Key.Code == sf::Key::Down) && (event.Type == sf::Event::KeyPressed))
	{
		if(buffer.down())
			inputfield->set(buffer.is_end() ? "" : buffer.val());
	}
	inputfield->handleEvent(event);
}

bool GuiTerminal::Buffer::up()
{
	bool not_start=act!=buff.begin();
	if(not_start)
		--act;
	return not_start;
}

std::string GuiTerminal::Buffer::val()
{
	return *act;
}
			
bool GuiTerminal::Buffer::is_end()
{
	return act==buff.end();
}
			
bool GuiTerminal::Buffer::down()
{
	bool not_end=act!=buff.end();
	if(not_end)
		++act;
	return not_end;
}

void GuiTerminal::Buffer::enter(std::string _val)
{
	if(buff.size()>size)
		buff.erase(buff.begin(), buff.begin() + (size/2));
	buff.push_back(_val);
	act=buff.end();
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
