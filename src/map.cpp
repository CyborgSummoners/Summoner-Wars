#include "map.hpp"
#include "mapgen.hpp"
#include <sstream>


namespace sum
{

Map::Map(sf::RenderWindow *_window) :
	Widget(_window,0,25,_window->GetWidth(),_window->GetHeight()*2/3),
	camera_x(0),
	camera_y(0)
{
	if (!map_image.LoadFromFile("resources/materials.png"))
	{
		std::cout<<"couldn't load resources/materials.png";
	}
	map_sprite.SetImage(map_image);
	map_sprite.SetBlendMode(sf::Blend::Multiply);
}

void Map::update(float tick)
{
	for(std::map<int,Robot>::iterator it=robots.begin(); it!=robots.end(); ++it )
		it->second.update(tick);
}

void Map::draw()
{
	using namespace stringutils;

	window->Draw(
		sf::Shape::Rectangle(x,y,width,height, sf::Color(255,255,255)));
	if(!map_layout.empty())
		for(int i=camera_y;i<map_layout.size() && i<CAMERA_HEIGHT;++i)
			for(int j=camera_x;j<map_layout[0].size() && j<CAMERA_WIDTH;++j)
			{
				map_sprite.SetX(x+j*SPRITE_SIZE);
				map_sprite.SetY(y+i*SPRITE_SIZE);
				switch(map_layout[i][j].type)
				{
					case Field::field:
						map_sprite.SetSubRect(
							sf::IntRect(
								0,0,SPRITE_SIZE,SPRITE_SIZE)
							);
						break;
					case Field::block:
						map_sprite.SetSubRect(
							sf::IntRect(
								SPRITE_SIZE,0,2*SPRITE_SIZE,SPRITE_SIZE)
							);
						break;
					default:
						map_sprite.SetSubRect(
							sf::IntRect(
								0,0,SPRITE_SIZE,SPRITE_SIZE)
							);
					break;
				}
				window->Draw(map_sprite);
				if(i==0 || (j==0 && i!=0))
				{
					number_text.SetX(i==0 ? x+j*SPRITE_SIZE : x);
					number_text.SetY(j==0 ? y+i*SPRITE_SIZE : y);
					number_text.SetSize(textSize);
					number_text.SetText(int_to_string(i==0 ? j : i));
					window->Draw(number_text);
				}
			}
	for(std::map<int,Robot>::iterator it=robots.begin(); it!=robots.end(); ++it )
		it->second.draw();
	for(std::map<int,Summoner>::iterator it=summoners.begin(); it!=summoners.end();++it)
		it->second.draw();
}

Map::Facing Map::armin_facing_converter(int facing)
{
	switch(facing)
	{
		case 0:
			return up;
		case 1:
			return left;
		case 2:
			return down;
		case 3:
			return right;
	}
}

void Map::parse_startmessage(const std::vector<std::string>& res) {
	using namespace stringutils;
	std::vector<Field> vtmp;
	int client_size(0);

	tick=string_to_float(res[0]);
	step_size=string_to_int(res[1]);
	steps_in_sec=step_size/tick;

	size_t map_y = string_to_int(res[3]);
	size_t map_x = string_to_int(res[2]);

	//mapdata
	sum::Terrain* terrain = new sum::Terrain[map_x*map_y];
	std::stringstream ss;
	ss.str(res[4]);
	Mapgen::reconstruct_from_dump(terrain, map_x, map_y, ss);
	//Mapgen::print_map(terrain, map_x, map_y, std::cout);

	for(size_t i=0;i<map_y;++i)
	{
		for(size_t j=0;j<map_x;++j) {
			vtmp.push_back(Field( terrain[i*map_x + j] == floor? Field::field : Field::block  )); //why oh why
		}
		map_layout.push_back(vtmp);
		vtmp.clear();
	}

	delete[] terrain;

	client_size=string_to_int(res[5]);

	for(int i=0;i<client_size;++i)
	{
		summoners.insert(std::pair<int,Summoner>(
			string_to_int(res[5+i*4]),
			Summoner(
				string_to_int(res[7+i*4]),
				string_to_int(res[6+i*4]),
				string_to_int(res[8+i*4]),
				string_to_int(res[9+i*4]),
				this
				)
			));
	}
}


void Map::update(const ServerMessage &message)
{
	using namespace stringutils;

	std::vector<std::string> res = message.get_parsed_msg();
	std::map<int,Robot>::iterator it;
	std::map<int,Summoner>::iterator it2;
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
							armin_facing_converter(string_to_int(res[2])),
							string_to_int(res[0])-1,
							string_to_int(res[3]),
							string_to_int(res[4]),
							this
							)
					)
				);
			break;
		case ServerMessage::start:
			parse_startmessage(res);
			break;

		case ServerMessage::death:
			it=robots.find(string_to_int(res[0]));
			if(it!=robots.end())
			{
				robots.erase(it);
				return;
			}
			it2=summoners.find(string_to_int(res[0]));
			if(it2!=summoners.end())
				summoners.erase(it2);
			break;

		case ServerMessage::turn:
			/*it=robots.find(string_to_int(res[0]));
			it->second.movings.push(
				Moving(
					armin_facing_converter(string_to_int(res[3])),
					string_to_float(res[4])/steps_in_sec,
					true
					)
				);

			break;*/
			break;
		case ServerMessage::move:
			if(string_to_int(res[5])!=0)
			{
				it=robots.find(string_to_int(res[0]));
				it->second.movings.push(
					Moving(
						coord_to_facing(
							string_to_int(res[1]),
							string_to_int(res[2]),
							string_to_int(res[3]),
							string_to_int(res[4])
							),
						string_to_int(res[6])/steps_in_sec,
						false
						)
					);
				it->second.map_x=string_to_int(res[1]);
				it->second.map_y=string_to_int(res[2]);
				it->second.facing=coord_to_facing(
							string_to_int(res[1]),
							string_to_int(res[2]),
							string_to_int(res[3]),
							string_to_int(res[4]));
			}
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

Map::Robot::Robot(
	int _ID,
	int _client_ID,
	Facing _facing,
	int _team,
	int _x,
	int _y,
	Map *_map) :
	Widget(_map->window,_x,_y,SPRITE_SIZE,SPRITE_SIZE),
	map_x(_x),
	map_y(_y),
	ID(_ID),
	client_ID(_client_ID),
	facing(_facing),
	team(_team),
	map(_map),
	speed(SPRITE_SIZE)
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
		(team%2)*SPRITE_SIZE,
		facing*SPRITE_SIZE,
		(team%2)*SPRITE_SIZE+SPRITE_SIZE,
		facing*SPRITE_SIZE+SPRITE_SIZE));
}

void Map::Robot::draw()
{
	window->Draw(sprite);
}

void Map::Robot::setToPos()
{
	setX(map->x + (map_x-map->camera_x)*SPRITE_SIZE);
	setY(map->y + (map_y-map->camera_y)*SPRITE_SIZE);
}

void Map::Robot::update(float tick)
{
	if(!movings.empty())
	{
		Moving &move=movings.back();
		if(!move.turn)
		{
			switch(move.way)
			{
				case down:
					y=y+(SPRITE_SIZE*tick+1)/(move.time);
				break;
				case left:
					x=x-(SPRITE_SIZE*tick+1)/(move.time);
				break;
				case right:
					x=x+(SPRITE_SIZE*tick+1)/(move.time);
				break;
				case up:
					y=y-(SPRITE_SIZE*tick+1)/(move.time);
				break;
			}
			sprite.SetX(x);
			sprite.SetY(y);
		}
		else
		{
			facing=move.way;
			sprite.SetSubRect(
				sf::IntRect(
					(team%2)*SPRITE_SIZE,
					facing*SPRITE_SIZE,
					(team%2)*SPRITE_SIZE+SPRITE_SIZE,
					facing*SPRITE_SIZE+SPRITE_SIZE));
			movings.pop();
			return;
		}

		//move.duration-=tick;
		move.duration-=tick*move.time;
		if(move.duration<=0 && !move.turn)
		{
			
				switch(move.way)
				{
					case down:
						map_y++;
					break;
					case up:
						map_y--;
					break;
					case left:
						map_x--;
					break;
					case right:
						map_x++;
					break;
				}

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
		2*SPRITE_SIZE,
		SPRITE_SIZE,
		3*SPRITE_SIZE));

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
