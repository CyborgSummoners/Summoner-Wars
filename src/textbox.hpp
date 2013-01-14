#ifndef TEXTBOX_HPP
#define TEXTBOX_HPP

#include "widget.hpp"

namespace sum
{

class TextBox : public Widget{

public:
	
	TextBox(
		sf::RenderWindow *_window,
		int _x,
		int _y,
		int _width,
		int _height,
		int _size=200
		);
	void draw();
	void add(std::string _text);
	std::vector<std::string> val();
	
private:

	std::vector<std::string> lines;
	int size;
	sf::String text;
	sf::String tmp;
	const int linesize;
	bool chopping;
	int chopping_size;
	
};

}
#endif
