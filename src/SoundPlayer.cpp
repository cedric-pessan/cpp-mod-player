
#include "SoundPlayer.hpp"

#include <iostream>
#include <SDL.h>

SoundPlayer::SoundPlayer()
{
   SDL_Init(SDL_INIT_AUDIO);
}

SoundPlayer::~SoundPlayer()
{
   SDL_Quit();
}

void SoundPlayer::play()
{
   std::cout << "TODO: SoundPlayer::play()" << std::endl;
}
