#ifndef WIDGET_HPP
#define WIDGET_HPP

#include "include.hpp"

namespace sum
{

class Widget{

public:

	Widget(sf::RenderWindow *_window,int _x=0,int _y=0);
	~Widget();
	
	virtual void draw()=0;
	virtual void handleEvent(sf::Event &event){}

protected:

	sf::RenderWindow *window;
	int x,y;
	static sf::Color textColor;
	static sf::Color bgColor;
	static int textSize;
};

}

#endif
