#include "textbox.hpp"

namespace sum
{

TextBox::TextBox(sf::RenderWindow *_window,int _x,int _y,int _width, int _height) :
Widget(_window, _x, _y),
width(_width), height(_height),
linesize(8)
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
	for(int i=0;i<lines.size();++i)
	{
		text.SetText(lines[i]);
		text.SetY(y + i*linesize);
		window->Draw(text);
	}
}

void TextBox::add(std::string _text)
{
	lines.push_back(_text);
}

std::vector<std::string> TextBox::val()
{
	return lines;
}

}
