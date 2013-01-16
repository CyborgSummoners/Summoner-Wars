#include "map.hpp"

namespace sum
{

Map::Map(sf::RenderWindow *_window) :
	Widget(_window,0,25,_window->GetWidth(),_window->GetHeight()*2/3)
{

}

Map::~Map()
{

}

void Map::draw()
{
		window->Draw(
			sf::Shape::Rectangle(x,y,width,height, sf::Color(128,128,128)));
}

}