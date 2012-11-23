#pragma once
#include "SFML/Window.hpp"
#include "SFML/Graphics.hpp"
#include "terminal.hpp"

namespace sum
{

class Game
{

public:
	static void Start();

private:
	static bool IsExiting();
	static void GameLoop();

	enum GameState { Uninitialized, ShowingSplash, Paused, ShowingMenu, Playing, Exiting };
  
	static GameState gameState;
	static sf::RenderWindow *mainWindow;
	static Terminal *terminal;
};

}
