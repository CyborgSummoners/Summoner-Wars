#include "guiterminal.hpp"
#include <iostream>

namespace sum{

GuiTerminal::GuiTerminal(sf::RenderWindow *_window) :
Widget(_window),
inputfield_size(25)
{
	window=_window;
	width=window->GetWidth();
	height=window->GetHeight()/3;
	x=0;
	y=window->GetHeight()-height;
	inputfield=new InputField(_window,5,_window->GetHeight()-inputfield_size);
	textbox=new TextBox(_window,5,y,width,height-inputfield_size);
}

GuiTerminal::~GuiTerminal()
{
	delete inputfield;
	delete textbox;
}

void GuiTerminal::draw()
{
	window->Draw(sf::Shape::Rectangle(
		0,
		window->GetHeight()-height ,
		width,
		window->GetHeight()-height + 2 ,
		textColor));
	inputfield->draw();
	textbox->draw();
}

void GuiTerminal::handleEvent(sf::Event &event)
{
	inputfield->handleEvent(event);
	if(event.Key.Code == sf::Key::Return)
	{
		std::string tmp=textbox->val() + '\n' + inputfield->val();
		textbox->set(tmp);
		inputfield->set("");
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
