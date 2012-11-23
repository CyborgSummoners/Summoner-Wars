#include "terminal.hpp"
#include <iostream>

namespace sum{

Terminal::Terminal(sf::RenderWindow &window)
{
	size=200;
	x=window.GetWidth()-size;
	y=window.GetWidth()-size;
}

void Terminal::Draw(sf::RenderWindow &window)
{
	window.Draw(sf::Shape::Rectangle(x,y,size,size,sf::Color(255,255,0)));
	input.SetX(400);
	input.SetY(400);
	window.Draw(input);
}

void Terminal::handleEvent(sf::Event &event)
{
	std::string fosom="a";
	if(event.Type == sf::Event::TextEntered)
	{
		char f=event.Key.Code;
		fosom[0]=f;
		input.SetText(fosom);
	}
}

}
