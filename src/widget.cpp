#include "widget.hpp"

namespace sum
{

Widget::Widget(sf::RenderWindow *_window,int _x,int _y,int _width, int _height) :
window(_window),
x(_x),
y(_y),
width(_width),
height(_height){}


void Widget::draw_borders()
{
	window->Draw(
		sf::Shape::Rectangle(x,y,x + width,y + bordersize ,textColor));
	window->Draw(
		sf::Shape::Rectangle(x,y,x - bordersize,y + height, textColor));
	window->Draw(
		sf::Shape::Rectangle(x, y + height - bordersize, x + width + bordersize, y + height, textColor));
	window->Draw(
		sf::Shape::Rectangle(x + width,y, x + width + bordersize , y + height,textColor));
}

sf::Color Widget::textColor(255,255,255);
sf::Color Widget::nameColor(128,128,128);
sf::Color Widget::bgColor(0,0,0);
int Widget::textSize(14);
int Widget::bordersize(2);

std::vector<std::string> Widget::string_explode(const std::string& str, const char& ch) {
    std::string next = "";
    std::vector<std::string> result;

    for (std::string::const_iterator it = str.begin(); it != str.end(); it++) {
    	if (*it == ch) {
    		if (next.length() > 0) {
    			result.push_back(next);
    			next = "";
    		}
    	} else {
    		next += *it;
    	}
    }

    return result;
}

}
