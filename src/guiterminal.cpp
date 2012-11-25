#include "guiterminal.hpp"
#include <iostream>

namespace sum{

GuiTerminal::GuiTerminal(sf::RenderWindow *_window) :
Widget(_window),
bgColor(0,213,0),
inputfield(_window,400,400)
{
	window=_window;
	width=window->GetWidth();
	height=window->GetHeight()/3;
	x=0;
	y=window->GetHeight()-height;
	input.SetColor(sf::Color(255,255,255));
}

void GuiTerminal::draw()
{
	inputfield.draw();
}

void GuiTerminal::handleEvent(sf::Event &event)
{
	inputfield.handleEvent(event);
}

GuiTerminal::Buffer::Buffer(int _size) :
strbuffer(_size),
act(0),head(0),
size(_size),
val()
{
}

void GuiTerminal::Buffer::up()
{
	if((act+1)%size!=head || strbuffer[(act+1)%size].empty())
		++act;
	val=strbuffer[act];
}

void GuiTerminal::Buffer::down()
{
	if(act!=head)
		--act;
	val=strbuffer[act];
}

void GuiTerminal::Buffer::enter()
{
	if(head==0)
		head=size-1;
	else
		--head;
	act=head;
	strbuffer[head]="";
	val=strbuffer[head];
}


}
