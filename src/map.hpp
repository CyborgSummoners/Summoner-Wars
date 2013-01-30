#ifndef MAP_HPP
#define MAP_HPP

#include "observable.hpp"
#include "widget.hpp"
#include <queue>
#include <map>

namespace sum
{

class Map : public Widget, public Observer<ServerMessage>
{

friend class Robot;

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
		Moving(Facing _way, int _duration=SPRITE_SIZE, bool _turn=false):
			way(_way), duration(_duration), turn(_turn){}

		Facing way;
		int duration;
		bool turn;
	};

	class Robot : public Widget
	{

	friend class Map;

	public:

		Robot(int _ID,int _team, int _x, int _y, Map *_map);

	private:

		int ID;
		int team;
		int map_x,map_y;
		float speed;
		Facing facing;
		sf::Sprite sprite;
		std::queue<Moving> movings;
		void setToPos();

	public:

		void draw();
		void update(float tick);

	private:

		static sf::Image robot_image;
		static bool initiated;
		const Map *map;
	};

	std::map<int,Robot> robots;

};

}

#endif
