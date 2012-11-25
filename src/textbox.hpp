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
	
private:

	int width, height;
	sf::String text;

};

}
#endif
