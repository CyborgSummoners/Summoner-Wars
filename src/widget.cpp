#include "widget.hpp"

namespace sum
{

Widget::Widget(sf::RenderWindow *_window,int _x,int _y) :
window(_window),
x(_x),
y(_y){}
Widget::~Widget()
{
}


sf::Color Widget::textColor(255,255,255);
sf::Color Widget::bgColor(0,0,0);
int Widget::textSize(20);


}
