#include "map.hpp"

namespace sum
{

Map::Map(sf::RenderWindow *_window) :
	Widget(_window,0,25,_window->GetWidth(),_window->GetHeight()*2/3),
	target(10,10,this)
{
	//target.shape.EnableFill(false);
	Robot rob(2,0,0,0,this);
	robots.push_back(rob);
	isCtrlDown = false;
}

void Map::handleEvent(sf::Event &event)
{
	if(event.Type == sf::Event::KeyPressed && event.Key.Code == sf::Key::LControl)
		isCtrlDown = true;
	if(event.Type == sf::Event::KeyReleased && event.Key.Code == sf::Key::LControl)
		isCtrlDown = false;
	if(isCtrlDown && event.Type == sf::Event::KeyPressed && event.Key.Code == sf::Key::Right)
		target.move(SPRITE_SIZE, 0);
	if(isCtrlDown && event.Type == sf::Event::KeyPressed && event.Key.Code == sf::Key::Left)
		target.move(-SPRITE_SIZE, 0);
	if(isCtrlDown && event.Type == sf::Event::KeyPressed && event.Key.Code == sf::Key::Up)
		target.move(0, -SPRITE_SIZE);
	if(isCtrlDown && event.Type == sf::Event::KeyPressed && event.Key.Code == sf::Key::Down)
		target.move(0, SPRITE_SIZE);
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
	shape = sf::Shape::Rectangle(_x,_y,_x+SPRITE_SIZE,_y+SPRITE_SIZE, sf::Color(0,255,0), 1, sf::Color(255,0,0));
	shape.EnableFill(false);
	this->x = _x;
	this->y = _y;
}

void Map::Targetfield::draw()
{
	window->Draw(shape);
}

void Map::Targetfield::move(int newx, int newy)
{
	x += newx;
	y += newy;
	shape.Move(newx, newy);
	//draw();
}

void Map::Targetfield::moveto(int newx, int newy)
{
	this->x = newx;
	this->y = newy;
	draw();
}

}
