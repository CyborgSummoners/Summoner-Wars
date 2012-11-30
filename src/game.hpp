#pragma once
#include "SFML/Window.hpp"
#include "SFML/Graphics.hpp"
#include "guiterminal.hpp"

namespace sum
{

class Game
{

public:
	static void Start();

private:
	static bool IsExiting();
	static void GameLoop();

	enum GameState { Uninitialized, Playing, Exiting };
  
	static GameState gameState;
	static sf::RenderWindow *mainWindow;
	static GuiTerminal *terminal;
};

}
