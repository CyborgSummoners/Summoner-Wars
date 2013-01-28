#ifndef MAP_HPP
#define MAP_HPP

#include "observable.hpp"
#include "widget.hpp"
#include <queue>

namespace sum
{

class Map : public Widget, public Observer<ServerMessage>
{

friend class Robot;
friend class Targetfield;

public:

	Map(sf::RenderWindow *_window);

	void draw();
	void update(const ServerMessage &message);
	void update(float tick);

private:
	enum Facing {down=0,left,right,up};
	static const int SPRITE_SIZE=32;

	struct Moving
	{
		Moving(Facing _way, int source_x, int source_y) :
			way(_way), dest_x(0),dest_y(0)
		{
			switch(_way)
			{
				case down:
					dest_x=source_x;
					dest_y=source_y+SPRITE_SIZE;
					break;
				case left:
					dest_x=source_x-SPRITE_SIZE;
					dest_y=source_y;
					break;
				case right:
					dest_x=source_x+SPRITE_SIZE;
					dest_y=source_y;
					break;
				case up:
					dest_x=source_x;
					dest_y=source_y-SPRITE_SIZE;
					break;
				default:
					break;
			}
		}

		Facing way;
		int dest_x;
		int dest_y;
		float speed;
	};

	class Robot : public Widget
	{
	public:

		Robot(int _ID,int _team, int _x, int _y, Map *_map);

	private:

		int ID;
		int team;
		Facing facing;
		sf::Sprite sprite;
		std::queue<Moving> movings;

	public:

		void draw();
		void update(float tick);

	private:

		static sf::Image robot_image;
		static bool initiated;
		const Map *map;
	};

	std::vector<Robot> robots;

	class Targetfield : public Widget
	{
	public:
		Targetfield(int _x, int _y, Map *_map);

		void draw();
		void move(int newx, int newy);
		void moveto(int newx, int newy);

	private:
		sf::Shape shape;
		const Map *map;
	};
	Targetfield target;
};
};

#endif
