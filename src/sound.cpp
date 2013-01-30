#include "sound.hpp"

namespace sum
{
    void Sound::PlaySound(std::string filename)
    {
        sf::SoundBuffer Buffer;
        if (!Buffer.LoadFromFile(filename))
            return;
        sf::Sound Sound(Buffer);
        Sound.Play();
        while (Sound.GetStatus() == sf::Sound::Playing)
        {
        }
    }
}
