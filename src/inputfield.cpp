#include "inputfield.hpp"

namespace sum
{

InputField::InputField(sf::RenderWindow *_window,int _x, int _y) :
Widget(_window,_x,_y)
{
	text.SetColor(textColor);
	text.SetX(x);
	text.SetY(y);
	text.SetSize(textSize);
}

void InputField::draw()
{
	window->Draw(text);
}
void InputField::handleEvent(sf::Event &event)
{
	if(event.Type == sf::Event::TextEntered)
	{
		tmp=event.Key.Code;
		value+=tmp;
		text.SetText(value);
	}
	if(event.Key.Code == sf::Key::Back)
	{
		value = value.substr(0,value.size()-1);
		text.SetText(value);
	}
}

std::string InputField::val()
{
	return value;
}

void InputField::set(std::string _value)
{
	value=_value;
}

}
