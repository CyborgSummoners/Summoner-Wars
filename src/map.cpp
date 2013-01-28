#include "map.hpp"

namespace sum
{

Map::Map(sf::RenderWindow *_window) :
	Widget(_window,0,25,_window->GetWidth(),_window->GetHeight()*2/3),
	target(0,0,this)
{
	Robot rob(2,0,0,0,this);
	//Targetfield target(0,0,this);

	robots.push_back(rob);
}

void Map::update(float tick)
{
	for(int i=0;i<robots.size();++i)
	{
		robots[i].update(tick);
	}
}

void Map::draw()
{
	window->Draw(
		sf::Shape::Rectangle(x,y,width,height, sf::Color(128,128,128)));
	for(int i=0;i<robots.size();++i)
		robots[i].draw();
	target.draw();
	//hogy a francba lehet kirajzoltatni az a h*lye shape-t?

}

void Map::update(const ServerMessage &message)
{
	switch(message.type)
	{
		case ServerMessage::unknown:
			std::cout<<"Map received unknown message: "<<message.msg;
			break;
		case ServerMessage::summon:
			break;
		case ServerMessage::move:

			break;
		case ServerMessage::shout:

			break;
		default:
			break;
	}
}

Map::Robot::Robot(int _ID,int _team, int _x, int _y, Map *_map) :
	Widget(_map->window,_x,_y,SPRITE_SIZE,SPRITE_SIZE),
	ID(_ID),
	team(_team),
	facing(down),
	map(_map)
{
	if(!initiated)
		if (!robot_image.LoadFromFile("resources/robots.png"))
		{
			std::cout<<"couldn't load resources/robots.png";
		}

	sprite.SetImage(robot_image);
	sprite.SetBlendMode(sf::Blend::Multiply);
	sprite.SetX(map->x + x*SPRITE_SIZE);
	sprite.SetY(map->y + y*SPRITE_SIZE);
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

void Map::Robot::update(float tick)
{
}

sf::Image Map::Robot::robot_image;
bool Map::Robot::initiated(false);

Map::Targetfield::Targetfield(int _x, int _y, Map *_map):
Widget(_map->window,_x,_y,20,20)
{
	shape = sf::Shape::Rectangle(_x,_y,_x+SPRITE_SIZE,_y+SPRITE_SIZE, sf::Color(0,255,0));
	this->x = _x;
	this->y = _y;
}

void Map::Targetfield::draw()
{
	window->Draw(shape);
}

void Map::Targetfield::move(int newx, int newy)
{
	this->x += newx;
	this->y += newy;
	window->Draw(this->shape);
}

void Map::Targetfield::moveto(int newx, int newy)
{
	this->x = newx;
	this->y = newy;
	window->Draw(this->shape);
}

}
