
#include "SoundPlayer.hpp"
#include "ModuleReader.hpp"

#include <iostream>
#include <SDL.h>
#include <memory>

namespace mods
{
   
   namespace
     {
        extern "C"
          {
             void s_ccallback(void* udata, Uint8* stream, int len)
               {
                  std::cout << "TODO: audio callback" << std::endl;
               }
          }
     }
   
   SoundPlayer::SoundPlayer()
     {
        int res = SDL_Init(SDL_INIT_AUDIO);
        check_init(res == 0, "sdl audio subsystem could not be initialized");
        SDL_AudioSpec spec;
        spec.freq = 44100;
        spec.format = AUDIO_S16;
        spec.channels = 2;
        spec.samples = 1024;
        spec.callback = s_ccallback;
        spec.userdata = this;
        
        res = SDL_OpenAudio(&spec, NULL);
        check_init(res >= 0, "audio device could not be opened");
     }
   
   SoundPlayer::~SoundPlayer()
     {
        SDL_CloseAudio();
        SDL_Quit();
     }
   
   void SoundPlayer::check_init(bool condition, const std::string& description)
     {
        if(!condition) throw SoundPlayerInitException(description);
     }
   
   void SoundPlayer::play(const std::shared_ptr<ModuleReader>& reader)
     {
        addReaderToPlayList();
        SDL_PauseAudio(0);
        waitUntilFinished();
        SDL_PauseAudio(1);
        removeOldestReaderFromPlayList();
     }
   
   void SoundPlayer::addReaderToPlayList()
     {
        std::cout << "TODO: SoundPlayer::addReaderToPlayList()" << std::endl;
     }
   
   void SoundPlayer::removeOldestReaderFromPlayList()
     {
        std::cout << "TODO: SoundPlyaer::removeOldestReaderFromPlayList()" << std::endl;
     }
   
   void SoundPlayer::waitUntilFinished()
     {
        std::cout << "TODO: SoundPlayer::waitUntilFinished()" << std::endl;
     }
   
   SoundPlayer::SoundPlayerInitException::SoundPlayerInitException(const std::string& reason)
     : _reason(reason)
     {
     }
   
   SoundPlayer::SoundPlayerInitException::~SoundPlayerInitException()
     {
     }
   
   const char* SoundPlayer::SoundPlayerInitException::what() const noexcept
     {
        return _reason.c_str();
     }

}
