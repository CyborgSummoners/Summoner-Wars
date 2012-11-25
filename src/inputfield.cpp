#include "inputfield.hpp"

namespace sum
{

InputField::InputField(sf::RenderWindow *_window,int _x, int _y) :
Widget(_window,_x,_y),
pos(0)
{
	text.SetX(x);
	text.SetY(y);
	cursor.SetX(x);
	cursor.SetY(y);
	text.SetSize(textSize);
	cursor.SetSize(textSize);
	
	//only works if i put this here.
	//interesting
	
	if(!gameFont.LoadFromFile("resources/FreeMono.ttf",50))
		std::cout<<"fosom";
}

void InputField::draw()
{
	text.SetFont(gameFont);
	text.SetColor(textColor);
	cursor.SetFont(gameFont);
	cursor.SetColor(textColor);
	cursy.insert(0,pos,' ');
	cursy.push_back('_');
	cursor.SetText(cursy);
	window->Draw(text);
	window->Draw(cursor);
	cursy="";
}
void InputField::handleEvent(sf::Event &event)
{
	if((event.Type == sf::Event::KeyPressed) && (event.Key.Code == sf::Key::Left))
	{
		if(pos>0)
			--pos;
	}
	if((event.Type == sf::Event::KeyPressed) && (event.Key.Code == sf::Key::Right))
	{
		if(pos<value.size())
			++pos;
	}
	if(event.Type == sf::Event::TextEntered)
	{
		if(event.Key.Code != sf::Key::Delete)
		{
			tmp=event.Key.Code;
			value.insert(value.begin()+pos,1,tmp);
			++pos;
			text.SetText(value);
		}
	}
	if(event.Key.Code == sf::Key::Back)
	{
		if(pos!=0)
		{
			--pos;
			value.erase(pos,1);
			text.SetText(value);
		}
	}
}

std::string InputField::val()
{
	return value;
}

void InputField::set(std::string _value)
{
	value=_value;
	pos=_value.size();
}

}
