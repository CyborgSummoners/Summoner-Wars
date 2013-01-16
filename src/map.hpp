#ifndef MAP_HPP
#define MAP_HPP

#include "observable.hpp"
#include "widget.hpp"

namespace sum
{

class Map : public Widget
{

public:

	Map(sf::RenderWindow *_window);
	~Map();

	void draw();

private:


};

}

#endif