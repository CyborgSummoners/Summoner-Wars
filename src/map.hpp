#ifndef MAP_HPP
#define MAP_HPP

#include "observable.hpp"
#include "widget.hpp"

namespace sum
{

class Map : public Widget, public Observer<ServerMessage>
{

public:

	Map(sf::RenderWindow *_window);

	void draw();
	void update(const ServerMessage &message);
	void update(float tick);

private:

	sf::Image robot_image;
	sf::Sprite robot_sprite;
	static const int SPRITE_SIZE=32;

	enum Facing {down=0,left,right,up};

	struct Robot
	{
		Robot(int _ID,int _team, int _x, int _y):
			ID(_ID),
			x(_x),
			y(_y),
			team(_team),
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
