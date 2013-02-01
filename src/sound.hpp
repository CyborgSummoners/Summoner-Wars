#ifndef SOUND_HPP
#define SOUND_HPP

#include "observable.hpp"
#include "servermessage.hpp"
#include <SFML/Audio.hpp>
#include <iomanip>
#include <iostream>
#include <string>
#include <queue>

namespace sum
{
	class Sound:public Observer<ServerMessage>
	{
	public:
		void update(const ServerMessage &message);
		static void PlaySound(void* data);
	private:
	};
}

#endif