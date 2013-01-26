#ifndef GAME_HPP
#define GAME_HPP

#include "connection.hpp"
#include "guiterminal.hpp"
#include "infobar.hpp"
#include "map.hpp"
#include "combatlog.hpp"

namespace sum
{

class Game
{

private:
	static Connection *connection; // connection to the server

public:
	static void Start(std::string server_ip, unsigned short server_port);

	static void SendRequest(const std::string& server_handle, const std::string& args = "");

private:
	static bool IsExiting();
	static void GameLoop();

	enum GameState { Uninitialized, Playing, Exiting };

	static GameState gameState;
	static sf::RenderWindow *mainWindow;
	static InfoBar *infobar;
	static GuiTerminal *terminal;
	static CombatLog *combat_log;
	static Map *map;
	static sf::Clock *sfclock;
};

}

#endif
