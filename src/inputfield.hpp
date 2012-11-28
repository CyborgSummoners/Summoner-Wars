#ifndef INPUTFIELD_HPP
#define INPUTFIELD_CPP

#include "widget.hpp"

namespace sum
{

class InputField : public Widget {

public:
	InputField(sf::RenderWindow *_window, int _x, int _y);
	
	virtual void draw();
	void handleEvent(sf::Event &event);
	std::string val();
	void set(std::string _value);
	void reset();
	
private:

	std::string value;
	std::string cursy;
	sf::String text;
	sf::String cursor;
	sf::String cursor_text;
	std::string curs_str;
	char tmp;
	int pos;
};

}
#endif
