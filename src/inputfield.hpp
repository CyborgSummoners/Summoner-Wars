#ifndef INPUTFIELD_HPP
#define INPUTFIELD_HPP

#include "widget.hpp"

namespace sum
{

class InputField : public Widget {

public:
	InputField(sf::RenderWindow *_window, int _x, int _y,int _width, int _height);
	
	virtual void draw();
	void handleEvent(sf::Event &event);
	std::string val();
	void set(std::string _value);
	void reset();
	void setX(int _x);
	void setWidth(int _width) {width=_width;}
	
private:

	std::string value;
	std::string cursy;
	sf::String text;
	sf::String cursor;
	sf::String cursor_text;
	std::string curs_str;
	char tmp;
	int pos;
	bool back_pushed;
};

}
#endif
