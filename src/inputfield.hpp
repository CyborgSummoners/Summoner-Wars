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
	
private:

	std::string value;
	sf::String text;
	char tmp;
};

}
#endif
