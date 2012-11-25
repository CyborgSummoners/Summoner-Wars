#include "guiterminal.hpp"
#include <iostream>

namespace sum{

GuiTerminal::GuiTerminal(sf::RenderWindow *_window) :
bgColor(0,213,0)
{
	window=_window;
	width=window->GetWidth();
	height=window->GetHeight()/3;
	x=0;
	y=window->GetHeight()-height;
	input.SetColor(sf::Color(255,255,255));
}

void GuiTerminal::Draw()
{
	window->Draw(sf::Shape::Rectangle(x,y,x+width,y+height,bgColor));
	input.SetX(400);
	input.SetY(400);
	window->Draw(input);
}

void GuiTerminal::handleEvent(sf::Event &event)
{
	if(event.Type == sf::Event::TextEntered)
	{
		char f=event.Key.Code;
		fosom+=f;
		input.SetText(fosom);
	}
	if(event.Key.Code == sf::Key::Back)
	{
		fosom = fosom.substr(0,fosom.size()-1);
		input.SetText(fosom);
	}
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
