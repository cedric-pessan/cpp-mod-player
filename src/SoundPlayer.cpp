
#include "SoundPlayer.hpp"
#include "ModuleReader.hpp"

#include <iostream>
#include <SDL.h>
#include <memory>
#include <mutex>

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
   
   void SoundPlayer::play(std::unique_ptr<ModuleReader> reader)
     {
        auto entryMutex = addReaderToPlayList(std::move(reader));
        SDL_PauseAudio(0);
        waitUntilFinished(entryMutex);
        SDL_PauseAudio(1);
        removeOldestReaderFromPlayList();
     }
   
   std::shared_ptr<std::mutex> SoundPlayer::addReaderToPlayList(std::unique_ptr<ModuleReader> reader)
     {
        std::lock_guard<std::mutex> lock(_playListMutex);
        SynchronizedReader r(std::move(reader), std::make_shared<std::mutex>());
        r.second->lock(); // we will unlock in callback when read is finished
        _playList.push_back(std::move(r));
        return _playList.back().second;
     }
   
   void SoundPlayer::removeOldestReaderFromPlayList()
     {
        std::cout << "TODO: SoundPlyaer::removeOldestReaderFromPlayList()" << std::endl;
     }
   
   void SoundPlayer::waitUntilFinished(const std::shared_ptr<std::mutex>& entryMutex)
     {
        entryMutex->lock();
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
