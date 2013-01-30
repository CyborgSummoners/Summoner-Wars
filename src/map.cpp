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
}

void Map::update(const ServerMessage &message)
{
	std::vector<std::string> res = message.get_parsed_msg();
	std::map<int,Robot>::iterator it;
	switch(message.type)
	{
		case ServerMessage::unknown:
			std::cout<<"Map received unknown message: "<<message.msg;
			break;
		case ServerMessage::summon:
			robots.insert(
				std::pair<int,Robot>
					(
						res[0],
						Robot(res[0],)
					)
				);
			break;
		case ServerMessage::move:
			//res = message.get_parsed_msg();
			//it = robots.find(res[0]);
			//it->second.movings.push(Moving())
			break;
		case ServerMessage::shout:

			break;
		default:
			break;
	}
}

Map::Robot::Robot(int _ID,int _team, int _x, int _y, Map *_map) :
	Widget(_map->window,_x,_y,SPRITE_SIZE,SPRITE_SIZE),
	map_x(_x),
	map_y(_y),
	ID(_ID),
	team(_team),
	facing(down),
	map(_map),
	speed(30)
{
	if(!initiated)
		if (!robot_image.LoadFromFile("resources/robots.png"))
		{
			std::cout<<"couldn't load resources/robots.png";
		}

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

sf::Image Map::Robot::robot_image;
bool Map::Robot::initiated(false);

}
