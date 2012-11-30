#include "game.hpp"

namespace sum
{

void Game::Start()
{
	if(gameState != Uninitialized)
		return;
	mainWindow->SetFramerateLimit(60);
	gameState = Game::Playing;
  
	while(!IsExiting())
	{
		GameLoop();
	}

	mainWindow->Close();
	delete mainWindow;
	delete terminal;
}

bool Game::IsExiting()
{
	return gameState == Game::Exiting;
}

void Game::GameLoop()
{
	sf::Event currentEvent;
	while(mainWindow->GetEvent(currentEvent))
	{
		switch(gameState)
		{
			case Game::Playing:
			{
				if(currentEvent.Type == sf::Event::Closed)
				{
					gameState = Game::Exiting;
				}
				terminal->handleEvent(currentEvent);
				break;
			}
			default:
			break;
		}
	}
	mainWindow->Clear();
	terminal->draw();
	mainWindow->Display();
}


Game::GameState Game::gameState = Uninitialized;
sf::RenderWindow *Game::mainWindow = new sf::RenderWindow(sf::VideoMode(1024,768,32),"Sumwar");
GuiTerminal *Game::terminal = new GuiTerminal(mainWindow);

}
