#include "map.hpp"

namespace sum
{

Map::Map(sf::RenderWindow *_window) :
	Widget(_window,0,25,_window->GetWidth(),_window->GetHeight()*2/3)
{
	if (!robot_image.LoadFromFile("resources/robots.png")){}
	robot_sprite.SetImage(robot_image);
	robot_sprite.SetBlendMode(sf::Blend::Multiply);

	Robot rob(2,0,0,0);

	robots.push_back(rob);


}

Map::~Map()
{

}

void Map::draw()
{
		window->Draw(
			sf::Shape::Rectangle(x,y,width,height, sf::Color(128,128,128)));
		draw_robots();
}

void Map::update(const ServerMessage &message)
{

}

void Map::draw_robots()
{
	for(int i=0;i<robots.size();++i)
	{
		robot_sprite.SetX(x+robots[i].x*SPRITE_SIZE);
		robot_sprite.SetY(y+robots[i].y*SPRITE_SIZE);
		robot_sprite.SetSubRect(
			sf::IntRect(
				robots[i].team*SPRITE_SIZE,
				robots[i].facing*SPRITE_SIZE,
				robots[i].team*SPRITE_SIZE+SPRITE_SIZE,
				robots[i].facing*SPRITE_SIZE+SPRITE_SIZE));
		window->Draw(robot_sprite);
	}
}

}
