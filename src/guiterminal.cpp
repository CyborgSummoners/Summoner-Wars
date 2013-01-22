#include "guiterminal.hpp"
#include <iostream>

namespace sum{

GuiTerminal::GuiTerminal(
	sf::RenderWindow *_window,
	std::string _player_name,
	int _width,
	int _height) :
Widget(_window),
player_name(_player_name),
inputfield_size(25),
frozen(false)
{
	window=_window;

	if(_width!=0)
		width=_width;
	else
		width=_window->GetWidth()-(_window->GetWidth()/3);
	if(_height!=0)
		height=_height;
	else
		height=_window->GetHeight()/3;

	term=new Terminal();
	x=5;
	y=window->GetHeight()-height;
	name_pwd.SetX(x);
	name_pwd.SetY(y+height-inputfield_size);
	name_pwd.SetSize(textSize);
	name_pwd.SetText(player_name + term->get_working_directory() + "$");
	inputfield=new InputField(
		_window,
		x+name_pwd.GetRect().GetWidth(),
		y+height-inputfield_size,
		width-x-name_pwd.GetRect().GetWidth()-x,
		inputfield_size);
	textbox=new TextBox(_window,x,y,width,height-(inputfield_size*2));

}

GuiTerminal::~GuiTerminal()
{
	delete inputfield;
	delete textbox;
	delete term;
}

void GuiTerminal::update(const ServerMessage &message)
{
	switch(message.type)
	{
		case ServerMessage::reply:
			frozen=false;
			break;
		default:
			break;
	}
}

void GuiTerminal::draw()
{
	inputfield->draw();
	name_pwd.SetColor(nameColor);
	window->Draw(name_pwd);
	textbox->draw();
}

void GuiTerminal::handleEvent(sf::Event &event)
{
	if(!frozen)
	{
		if((event.Key.Code == sf::Key::Return) && (event.Type == sf::Event::KeyPressed))
		{
			buffer.enter(inputfield->val());
			std::string term_user=player_name + term->get_working_directory() + "$";
			textbox->add(term_user+inputfield->val());
			std::vector<std::string> ret = string_explode(term->command(inputfield->val()), '\n');

			if(ret[0] == "//") frozen=true;

			term_user=player_name + term->get_working_directory() + "$";
			name_pwd.SetText(term_user);
			inputfield->setX(x + name_pwd.GetRect().GetWidth());
			inputfield->setWidth(width-x-name_pwd.GetRect().GetWidth()-x);

			if(frozen)
				textbox->add("Waiting for server to reply...");
			else
			{
				for(int i=0; i<ret.size(); ++i)
					textbox->add(ret[i]);
			}

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
	if(buff.size()>static_cast<size_t>(size))
		buff.erase(buff.begin(), buff.begin() + (size/2));
	buff.push_back(_val);
	act=buff.end();
}


}
