#include "inputfield.hpp"

namespace sum
{

InputField::InputField(sf::RenderWindow *_window,int _x, int _y,int _width, int _height) :
Widget(_window,_x,_y,_width,_height),
pos(0),
back_pushed(false)
{
	text.SetX(x);
	text.SetY(y);
	cursor.SetX(x);
	cursor.SetY(y);
	cursor_text.SetX(x);
	cursor_text.SetY(y);
	text.SetSize(textSize);
	cursor.SetSize(textSize);
	cursor_text.SetSize(textSize);
	cursor.SetText("_");
}

void InputField::draw()
{
	text.SetColor(textColor);
	cursor.SetColor(textColor);
	cursor.SetStyle(sf::String::Bold);
	//cursor.SetColor(textColor);
	//cursy.insert(0,pos,' ');
	//cursy.push_back('_');
	//cursor.SetText(cursy);
	window->Draw(text);
	window->Draw(cursor);
	curs_str=value.substr(0,pos);
	cursor_text.SetText(curs_str);
	cursor.SetX(x+cursor_text.GetRect().GetWidth());
	//cursy="";
}

void InputField::setX(int _x)
{
	x=_x;
	text.SetX(_x);
	cursor.SetX(_x);
	cursor_text.SetX(_x);
}

void InputField::handleEvent(sf::Event &event)
{
	if((event.Type == sf::Event::KeyPressed) && (event.Key.Code == sf::Key::Left))
	{
		if(pos>0)
			--pos;
		return;
	}
	if((event.Type == sf::Event::KeyPressed) && (event.Key.Code == sf::Key::Right))
	{
		if(static_cast<size_t>(pos)<value.size())
			++pos;
		return;
	}
	if( event.Type == sf::Event::KeyPressed && event.Key.Code == sf::Key::Home ) {
		pos = 0;
		return;
	}
	if( event.Type == sf::Event::KeyPressed && event.Key.Code == sf::Key::End ) {
		pos = value.size();
		return;
	}


	if((event.Type == sf::Event::KeyPressed) && (event.Key.Code == sf::Key::Back))
	{
		back_pushed=true;
		return;
	}

	if((event.Type == sf::Event::KeyReleased) && (event.Key.Code == sf::Key::Back))
	{
		back_pushed=false;
		return;
	}

	if(back_pushed)
	{
		if(pos!=0)
		{
			--pos;
			value.erase(pos,1);
			text.SetText(value);
		}
		return;
	}
	if(event.Type == sf::Event::TextEntered)
	{
		if(text.GetRect().GetWidth() < width)
		{
			tmp=event.Key.Code;
			value.insert(value.begin()+pos,1,tmp);
			text.SetText(value);
			++pos;
			return;
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
	text.SetText(value);
}

void InputField::reset()
{
	value="";
	pos=0;
	text.SetText(value);
}

}
