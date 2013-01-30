#include "map.hpp"

namespace sum
{

Map::Map(sf::RenderWindow *_window) :
	Widget(_window,0,25,_window->GetWidth(),_window->GetHeight()*2/3)
{
}

void Map::update(float tick)
{
	for(std::map<int,Robot>::iterator it=robots.begin(); it!=robots.end(); ++it )
		it->second.update(tick);
}

void Map::draw()
{
	window->Draw(
		sf::Shape::Rectangle(x,y,width,height, sf::Color(128,128,128)));
	for(std::map<int,Robot>::iterator it=robots.begin(); it!=robots.end(); ++it )
		it->second.draw();
	for(std::map<int,Summoner>::iterator it=summoners.begin(); it!=summoners.end();++it)
		it->second.draw();
}

void Map::update(const ServerMessage &message)
{
	using namespace stringutils;

	std::vector<std::string> res = message.get_parsed_msg();
	std::map<int,Robot>::iterator it;
	std::vector<Field> vtmp;
	int client_size(0);
	switch(message.type)
	{
		case ServerMessage::unknown:
			std::cout<<"Map received unknown message: "<<message.msg;
			break;
		case ServerMessage::summon:
			robots.insert(
				std::pair<int,Robot>
					(
						string_to_int(res[1]),
						Robot(
							string_to_int(res[1]),
							string_to_int(res[0]),
							string_to_int(res[0])-1,
							string_to_int(res[2]),
							string_to_int(res[3]),
							this
							)
					)
				);
			break;
		case ServerMessage::start:
			tick=string_to_float(res[0]);
			step_size=string_to_int(res[1]);

			for(int i=0;i<string_to_int(res[3]);++i)
			{
				for(int j=0;j<string_to_int(res[2]);++j)
					vtmp.push_back(Field());
				map_layout.push_back(vtmp);
				vtmp.clear();
			}

			client_size=string_to_int(res[4]);

			for(int i=0;i<client_size;++i)
			{
				summoners.insert(std::pair<int,Summoner>(
					string_to_int(res[5+i*4]),
					Summoner(
						string_to_int(res[6+i*4]),
						string_to_int(res[5+i*4]),
						string_to_int(res[7+i*4]),
						string_to_int(res[8+i*4]),
						this
						)
					));
			}


			break;

		case ServerMessage::move:

			it=robots.find(string_to_int(res[0]));
			it->second.movings.push(
				Moving(
					coord_to_facing(
						string_to_int(res[1]),
						string_to_int(res[2]),
						string_to_int(res[3]),
						string_to_int(res[4])
						)
					)
				);
			
			break;
		case ServerMessage::shout:

			break;
		default:
			break;
	}
}

Map::Facing Map::coord_to_facing(int x1, int y1, int x2, int y2)
{
	bool sidemove((x1-x2)!=0);
	bool verticalmove((y1-y2)!=0);
	if(sidemove && verticalmove)
		return unknown;
	if(!sidemove)
		if((y1-y2) < 0)
			return down;
		else
			return up;
	else
		if((x1-x2) < 0)
			return right;
		else
			return left;
}

Map::Robot::Robot(int _ID,int _client_ID,int _team, int _x, int _y, Map *_map) :
	Widget(_map->window,_x,_y,SPRITE_SIZE,SPRITE_SIZE),
	map_x(_x),
	map_y(_y),
	ID(_ID),
	client_ID(_client_ID),
	team(_team),
	facing(down),
	map(_map),
	speed(3)
{
	if(!initiated)
	{
		if (!robot_image.LoadFromFile("resources/robots.png"))
		{
			std::cout<<"couldn't load resources/robots.png";
		}
	}
	initiated=true;

	setX(map->x + x*SPRITE_SIZE);
	setY(map->y + y*SPRITE_SIZE);

	sprite.SetImage(robot_image);
	sprite.SetBlendMode(sf::Blend::Multiply);
	sprite.SetX(x);
	sprite.SetY(y);
	sprite.SetSubRect(
	sf::IntRect(
		team*SPRITE_SIZE,
		facing*SPRITE_SIZE,
		team*SPRITE_SIZE+SPRITE_SIZE,
		facing*SPRITE_SIZE+SPRITE_SIZE));
}

void Map::Robot::draw()
{
	window->Draw(sprite);
}

void Map::Robot::setToPos()
{
	setX(map->x + map_x*SPRITE_SIZE);
	setY(map->y + map_y*SPRITE_SIZE);
}

void Map::Robot::update(float tick)
{
	if(!movings.empty())
	{
		Moving &move=movings.back();
		switch(move.way)
		{
			case down:
				y=y+speed*tick;
			break;
			case left:
				x=x-speed*tick;
			break;
			case right:
				x=x+speed*tick;
			break;
			case up:
				y=y-speed*tick;
			break;
		}
		move.duration-=speed*tick;
		if(move.duration<=0)
		{
			setToPos();
			movings.pop();
		}
	}
}

Map::Summoner::Summoner(int _ID,int _client_ID, int _x, int _y, Map *_map) :
		Widget(_map->window, _x,_y, SPRITE_SIZE, SPRITE_SIZE),
		ID(_ID),
		client_ID(_client_ID),
		map_x(_x),
		map_y(_y),
		map(_map)
{
	/*if(!initiated)
		if (!summoner_image.LoadFromFile("resources/summoners.png"))
		{
			std::cout<<"couldn't load resources/summoners.png";
		}
		*/
	if(!initiated)
		if (!summoner_image.LoadFromFile("resources/robots.png"))
		{
			std::cout<<"couldn't load resources/robots.png";
		}
	initiated=true;

	setX(map->x + x*SPRITE_SIZE);
	setY(map->y + y*SPRITE_SIZE);

	sprite.SetImage(summoner_image);
	sprite.SetBlendMode(sf::Blend::Multiply);
	sprite.SetX(x);
	sprite.SetY(y);
	sprite.SetSubRect(
	sf::IntRect(
		0,
		3*SPRITE_SIZE,
		SPRITE_SIZE,
		4*SPRITE_SIZE));

}

void Map::Summoner::draw()
{
	window->Draw(sprite);
}

void Map::Summoner::update(float tick)
{

}

sf::Image Map::Robot::robot_image;
bool Map::Robot::initiated(false);

sf::Image Map::Summoner::summoner_image;
bool Map::Summoner::initiated(false);

}
