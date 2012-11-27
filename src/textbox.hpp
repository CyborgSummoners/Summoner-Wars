#ifndef TEXTBOX_HPP
#define TEXTBOX_HPP

#include "widget.hpp"

namespace sum
{

class TextBox : public Widget{

public:
	
	TextBox(sf::RenderWindow *_window,int _x,int _y,int _width, int _height);
	void draw();
	void add(std::string _text);
	std::vector<std::string> val();
	
private:

	int width, height;
	std::vector<std::string> lines;
	sf::String text;
	const int linesize;
};

}
#endif
