#include "game.hpp"

namespace sum
{

void Game::Start()
{
  if(_gameState != Uninitialized)
    return;

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
          _mainWindow.Clear(sf::Color(255,0,0));
          _mainWindow.Display();
        
          if(currentEvent.Type == sf::Event::Closed)
            {
              _gameState = Game::Exiting;
            }
          break;
        }
    }
  }
}


Game::GameState Game::_gameState = Uninitialized;
sf::RenderWindow Game::_mainWindow;

}
