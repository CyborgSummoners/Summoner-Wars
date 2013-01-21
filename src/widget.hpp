#ifndef WIDGET_HPP
#define WIDGET_HPP

#include "include.hpp"
#include "servermessage.hpp"
#include "observer.hpp"
#include "observable.hpp"

namespace sum
{

class Widget
{

public:

	Widget(
		sf::RenderWindow *_window,
		int _x=0,
		int _y=0,
		int _width=0,
		int _height=0
		);
	virtual ~Widget(){}
	
	virtual void draw()=0;
	virtual void handleEvent(sf::Event &event){}
	virtual void setX(int _x) {x=_x;}
	virtual void setY(int _y) {y=_y;}
	virtual void setWidth(int _width) {width=_width;}
	virtual void setHeight(int _height) {height=_height;}
	int getX() {return x;}
	int getY() {return y;}
	int getWidth() {return width;}
	int getHeight() {return height;} 
 
protected:

	sf::RenderWindow *window;
	int x,y,width, height;
	static sf::Color textColor;
	static sf::Color nameColor;
	static sf::Color bgColor;
	static int bordersize;
	static int textSize;

	void draw_borders();

	std::vector<std::string> string_explode(const std::string& str, const char& ch);
};

}

#endif
