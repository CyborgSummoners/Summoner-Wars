#include "terminal.hpp"
#include <iostream>

namespace sum{

Terminal::Terminal(sf::RenderWindow *_window) :
bgColor(0,213,0)
{
	window=_window;
	width=window->GetWidth();
	height=window->GetHeight()/3;
	x=0;
	y=window->GetHeight()-height;
	input.SetColor(sf::Color(255,255,255));
}

void Terminal::Draw()
{
	window->Draw(sf::Shape::Rectangle(x,y,x+width,y+height,bgColor));
	input.SetX(400);
	input.SetY(400);
	window->Draw(input);
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

Terminal::Buffer::Buffer(int _size) :
strbuffer(_size),
act(0),head(0),
size(_size)
{
	val=&strbuffer[head];
}

void Terminal::Buffer::up()
{
	if((act+1)%size!=head || strbuffer[(act+1)%size].empty())
		++act;
	val=&strbuffer[act];
}

void Terminal::Buffer::down()
{
	if(act!=head)
		--act;
	val=&strbuffer[act];
}

void Terminal::Buffer::enter()
{
	if(head==0)
		head=size-1;
	else
		--head;
	act=head;
	strbuffer[head]="";
	val=&strbuffer[head];
}


}
