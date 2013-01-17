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

	sf::Image robot_image;
	sf::Sprite robot_sprite;
	static const int SPRITE_SIZE=32;

	enum Facing {down=0,left,right,up};

	struct Robot
	{
		Robot(int _ID,int _team, int _x, int _y):
			ID(_ID),
			team(_team),
			x(_x),
			y(_y),
			facing(down){}

		int ID;
		int x,y;
		int team;
		Facing facing;
	};

	std::vector<Robot> robots;

	void draw_robots();

};

}

#endif