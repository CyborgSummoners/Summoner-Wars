#include "game.hpp"

namespace sum
{

void Game::Start()
{
  if(_gameState != Uninitialized)
    return;
  _mainWindow.SetFramerateLimit(60);
  _mainWindow.Create(sf::VideoMode(800,600,32),"Sumwar");
  _gameState = Game::Playing;
  
  while(!IsExiting())
  {
    GameLoop();
  }

  _mainWindow.Close();
}

bool Game::IsExiting()
{
  return _gameState == Game::Exiting;
}

void Game::GameLoop()
{
  sf::Event currentEvent;
  while(_mainWindow.GetEvent(currentEvent))
  {
    switch(_gameState)
    {
      case Game::Playing:
        {
          if(currentEvent.Type == sf::Event::Closed)
            {
              _gameState = Game::Exiting;
            }
          break;
        }
    }
  }
  _mainWindow.Clear();
  _mainWindow.Display();
}


Game::GameState Game::_gameState = Uninitialized;
sf::RenderWindow Game::_mainWindow;

}
