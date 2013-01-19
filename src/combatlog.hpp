#ifndef COMBATLOG_HPP
#define COMBATLOG_HPP

#include "textbox.hpp"

namespace sum
{
	
class CombatLog : public TextBox, public Observer<ServerMessage>
{

public:

	CombatLog(
	sf::RenderWindow *_window,
	int _x,
	int _y,
	int _width,
	int _height,
	int _size=200
	) : 
	TextBox(_window,_x,_y,_width,_height){}

	void update(ServerMessage &message);

};

}

#endif