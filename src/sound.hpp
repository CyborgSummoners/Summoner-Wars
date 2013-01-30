#ifndef SOUND_HPP
#define SOUND_HPP

#include <SFML/Audio.hpp>
#include <iomanip>
#include <iostream>
#include <string>

namespace sum
{
	class Sound
	{
	public:
		static void PlaySound(std::string filename);
	private:
	};
}

#endif