#include "game.hpp"
#include "bytecode.hpp"
#include "util/debug.hpp"
#include "parser.hpp"
#include <cstdio>
#include <string>
#include <sstream>
#include <SFML/Network.hpp>

namespace sum
{
void Game::Start(std::string server_ip, unsigned short server_port)
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
	combat_log = new CombatLog(
		mainWindow,
		terminal->getX() + terminal->getWidth() + 5,
		terminal->getY(),
		mainWindow->GetWidth() - terminal->getWidth() - 10,
		terminal->getHeight() - 30
		);

	infobar = new InfoBar(mainWindow, "testplaya            ------ INFOBAR -------");
	map = new Map(mainWindow);

	//CREATING CONNECTION WITH OBSERVERS

	std::vector<Observer<ServerMessage>*> obss;
	obss.push_back(combat_log);
	obss.push_back(infobar);
	obss.push_back(map);

	connection = new Connection(obss);

	//TESTLINES

	for(int i=0; i<20 ;++i)
		combat_log->add("combat log line");

	mainWindow->SetFramerateLimit(10);
	gameState = Game::Playing;

	// connecting:
	bool success = false;
	if( connection->connect(server_ip, server_port) ) {

		//load scripts
		sf::Packet scripts = Parser::packetize_scripts_from_file("script-samples/functions.summ");

		if( connection->send_scripts(scripts) ) {
			connection->listen();
			combat_log->add( "Connected to " + connection->get_address() );
			success = true;
		}
	}

	if(!success) {
		std::stringstream ss;
		ss << "Could not connect to " << server_ip << ":" << server_port;
		combat_log->add(ss.str());
	}

	sfclock = new sf::Clock();

	while(!IsExiting())
	{
		GameLoop();
	}

	connection->disconnect();

	mainWindow->Close();
	delete mainWindow;
	delete terminal;
	delete combat_log;
	delete infobar;
	delete map;
	delete connection;
	delete sfclock;
}

void Game::SendRequest(const std::string& server_handle, const std::string& args) {
	sf::Packet packet;
	packet << server_handle;
	packet << args;
	connection->send(packet);
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

	float tick = sfclock->GetElapsedTime();
	map->update(tick);
	sfclock->Reset();

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
CombatLog *Game::combat_log = NULL;
InfoBar *Game::infobar = NULL;
Map *Game::map = NULL;
Connection *Game::connection = NULL;
sf::Clock *Game::sfclock = NULL;

}
