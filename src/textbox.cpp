#include "textbox.hpp"

namespace sum
{

TextBox::TextBox(sf::RenderWindow *_window,int _x,int _y,int _width, int _height) :
Widget(_window, _x, _y),
width(_width), height(_height)
{
	if(!gameFont.LoadFromFile("resources/FreeMono.ttf",50))
	std::cout<<"fosom";
	text.SetX(x);
	text.SetY(y);
	text.SetSize(textSize);
}

void TextBox::draw()
{
	text.SetColor(textColor);
	window->Draw(text);
}

void TextBox::set(std::string _text)
{
	text.SetText(_text);
}

}
