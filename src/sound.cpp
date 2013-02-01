#include "sound.hpp"
#include <string>
namespace sum
{

    void startthread(std::string fname) {
        sf::Thread Thread1(&(Sound::PlaySound),&fname);
        Thread1.Launch();
    }

    void Sound::update(const ServerMessage &message)
    {
        std::string fname;
        std::cout << "ohai from sound" << std::endl;
        switch(message.type)
        {
            case ServerMessage::death:
                startthread("resources/asta_la_vista.wav");
                break;
            case ServerMessage::move:
                startthread("resources/step.wav");
            break;
            default:
            break;
        }
    }

    void Sound::PlaySound(void* data)
    {

        std::string* filename = static_cast<std::string*>(data);
        sf::SoundBuffer Buffer;
        if (!Buffer.LoadFromFile(*filename))
            return;
        sf::Sound Sound(Buffer);
        Sound.Play();
        while (Sound.GetStatus() == sf::Sound::Playing)
        {
        }
    }
}