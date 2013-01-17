#include "game.hpp"
#include "util/debug.hpp"
#include <cstdio>
#include <iostream>

namespace sum
{

sf::SocketTCP Game::connection = sf::SocketTCP();

void Game::Start(sf::IPAddress server_ip, unsigned short server_port) {
	if(gameState != Uninitialized) return;
	debugf("Trying to connect to server at %s:%d\n", server_ip.ToString().c_str(), server_port);
	connection.SetBlocking(true);
	std::cerr << connection.Connect(server_port, server_ip, 4.0f) << std::endl;
	if(connection.Connect(server_port, server_ip, 4.0f) != sf::Socket::Done) {	//timeout of whopping 4 seconds
		printf("Could not connect to remote server, exiting\n");
		exit(0);
	} else debugf("Connected.\n");

	Start();
}


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

	infobar = new InfoBar(mainWindow, "testplaya            ------ INFOBAR -------");
	map = new Map(mainWindow);

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
	delete combat_log;
	delete infobar;
	delete map;

	if(connection.IsValid()) connection.Close();
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
		if(currentEvent.Type==10 || currentEvent.Type==11 || currentEvent.Type==12)
			return;

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
	map->draw();

	mainWindow->Display();
}


Game::GameState Game::gameState = Uninitialized;
sf::RenderWindow *Game::mainWindow = NULL;
GuiTerminal *Game::terminal = NULL;
TextBox *Game::combat_log = NULL;
InfoBar *Game::infobar = NULL;
Map *Game::map = NULL;

}
