#ifndef GAME_HPP
#define GAME_HPP

#include <SFML/Network.hpp>
#include "guiterminal.hpp"
#include "infobar.hpp"

namespace sum
{

class Game
{
private:
	static sf::SocketTCP connection; // connection to the server

public:
	static void Start();
	static void Start(sf::IPAddress server_ip, unsigned short server_port);

private:
	static bool IsExiting();
	static void GameLoop();

	enum GameState { Uninitialized, Playing, Exiting };

	static GameState gameState;
	static sf::RenderWindow *mainWindow;
	static InfoBar *infobar;
	static GuiTerminal *terminal;
	static TextBox *combat_log;
};

}

#endif
