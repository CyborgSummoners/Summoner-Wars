#ifndef INFOBAR_HPP
#define INFOBAR_HPP

#include "widget.hpp"

namespace sum
{

class InfoBar : public Widget, public Observer<ServerMessage>{

public:

	InfoBar(sf::RenderWindow *_window, std::string _player_name);
	void draw();
	void update(const ServerMessage &message);

private:

	std::string player_name;
	std::string ID;
	std::vector<std::string> summons;
	sf::String text;

};


}


#endif
