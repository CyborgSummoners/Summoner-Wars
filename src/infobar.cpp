#include "infobar.hpp"

namespace sum
{

InfoBar::InfoBar(sf::RenderWindow *_window, std::string _player_name) :
Widget(_window,5,0,_window->GetWidth(),25),
player_name(_player_name)
{
	text.SetX(x);
	text.SetY(y);
	text.SetSize(textSize);
	text.SetText(player_name);
}

void InfoBar::draw()
{
	text.SetColor(textColor);
	window->Draw(text);
}

void InfoBar::update(const ServerMessage &message)
{
	using namespace stringutils;

	std::vector<std::string> res = message.get_parsed_msg();

	switch(message.type)
	{
		case ServerMessage::unknown:
			std::cout<<"infobar received unknown message: "<< message.msg << std::endl;
		break;
		case ServerMessage::start:
			///ID=
		break;
		case ServerMessage::summon:

		break;
		default:
			//std::cout << "wtf" << std::endl;
		break;
	}
}

}
