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

	enum Facing {down=0,left,right,up,unknown};
	Facing coord_to_facing(int x1, int y1, int x2, int y2);

	struct Field
	{
		enum Type {field, block};
		Field(Type _type=field) : type(_type){}

		Type type;
	};

	static const int SPRITE_SIZE=32;
	std::vector<std::vector<Field> > map_layout;

	struct Moving
	{
		Moving(Facing _way, float _duration=SPRITE_SIZE, bool _turn=false):
			way(_way), duration(_duration), turn(_turn){}

		Facing way;
		float duration;
		bool turn;
	};

	class Summoner : public Widget
	{

	friend class Map;

	public:

		Summoner(int _ID, int _client_ID,int _x, int _y, Map *_map);

	private:

		int ID;
		int client_ID;
		int map_x;
		int map_y;
		sf::Sprite sprite;

	public:

		void draw();
		void update(float tick);

	private:

		static sf::Image summoner_image;
		static bool initiated;
		const Map *map;

	};

	class Robot : public Widget
	{

	friend class Map;

	public:

		Robot(int _ID, int _client_ID,int _team, int _x, int _y, Map *_map);

	private:

		int ID;
		int client_ID;
		int team;
		int map_x;
		int map_y;
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

	std::map<int,Robot> robots; // key::actor ID
	std::map<int,Summoner> summoners; // key::client ID
	float tick;
	int step_size;

};

}

#endif
