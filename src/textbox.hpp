#ifndef TEXTBOX_HPP
#define TEXTBOX_HPP

#include "widget.hpp"

namespace sum
{

class TextBox : public Widget{

public:
	
	TextBox(sf::RenderWindow *_window,int _x,int _y,int _width, int _height);
	void draw();
	void set(std::string _text);
	std::string val();
	
private:

	int width, height;
	std::string value;
	sf::String text;

};

}
#endif
