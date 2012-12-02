#include "game.hpp"

namespace sum
{

void Game::Start()
{
	if(gameState != Uninitialized)
		return;
	gameState = Uninitialized;
	mainWindow = new sf::RenderWindow(sf::VideoMode(1024,768,32),"Sumwar");
	terminal = new GuiTerminal(
		mainWindow,
		"testplaya",
		mainWindow->GetWidth()-(mainWindow->GetWidth()/3),
		mainWindow->GetHeight()/3
		);
	combat_log = new TextBox(
		mainWindow,
		terminal->getX() + terminal->getWidth() + 5,
		terminal->getY(),
		mainWindow->GetWidth() - terminal->getWidth() - 10,
		terminal->getHeight() - 30
		);

	infobar = new InfoBar(mainWindow, "testplaya");

	//TESTLINES

	for(int i=0; i<20 ;++i)
		combat_log->add("combat log line");



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
	combat_log->draw();
	infobar->draw();

	mainWindow->Draw(
		sf::Shape::Rectangle(0,25,mainWindow->GetWidth(),
			mainWindow->GetHeight()*2/3, sf::Color(128,128,128)));

	mainWindow->Display();
}


Game::GameState Game::gameState = Uninitialized;
sf::RenderWindow *Game::mainWindow = NULL;
GuiTerminal *Game::terminal = NULL;
TextBox *Game::combat_log = NULL;
InfoBar *Game::infobar = NULL;

}
