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

	template<typename T>
	void remove_last_line_if(const T& predicate) {
		if(!lines.empty() && predicate(lines.back())) lines.pop_back();
	}

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
