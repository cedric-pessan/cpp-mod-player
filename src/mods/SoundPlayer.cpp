
#include "mods/SoundPlayer.hpp"
#include "mods/ModuleReader.hpp"

#include <SDL.h>
#include <iostream>
#include <memory>
#include <mutex>

namespace mods
{
   
   extern "C"
     {
        void SoundPlayer::s_ccallback(void* udata, Uint8* stream, int len)
          {
             auto sp = static_cast<SoundPlayer*>(udata);
             sp->callback(stream, len);
          }
     }
   
   SoundPlayer::SoundPlayer(ModuleInfoCallback moduleInfoCb, ModuleProgressCallback moduleProgressCb)
     : _moduleInfoCb(moduleInfoCb),
     _moduleProgressCb(moduleProgressCb)
       {
          int res = SDL_Init(SDL_INIT_AUDIO);
          checkInit(res == 0, "sdl audio subsystem could not be initialized");
          SDL_AudioSpec spec;
          spec.freq = 44100;
          spec.format = AUDIO_S16;
          spec.channels = 2;
          spec.samples = 1024;
          spec.callback = s_ccallback;
          spec.userdata = this;
          
          res = SDL_OpenAudio(&spec, nullptr);
          checkInit(res >= 0, "audio device could not be opened");
       }
   
   SoundPlayer::~SoundPlayer()
     {
        SDL_CloseAudio();
        SDL_Quit();
     }
   
   void SoundPlayer::checkInit(bool condition, const std::string& description) const
     {
        if(!condition)
          {
             throw SoundPlayerInitException(description);
          }
     }
   
   void SoundPlayer::play(ModuleReader::ptr reader)
     {
        sendModuleInfo(*reader);
        auto& entry = addReaderToPlayList(std::move(reader));
        SDL_PauseAudio(0);
        waitUntilFinished(entry);
        SDL_PauseAudio(1);
        removeOldestReaderFromPlayList();
     }
   
   const SoundPlayer::SynchronizedReader& SoundPlayer::addReaderToPlayList(ModuleReader::ptr reader)
     {
        std::lock_guard<std::mutex> lock(_playListMutex);
        SynchronizedReader r(std::move(reader), std::make_shared<std::mutex>());
        r.second->lock(); // we will unlock in callback when read is finished
        _playList.push_back(std::move(r));
        return _playList.back();
     }
   
   void SoundPlayer::removeOldestReaderFromPlayList()
     {
        std::lock_guard<std::mutex> lock(_playListMutex);
        _playList.pop_front();
     }
   
   void SoundPlayer::waitUntilFinished(const SynchronizedReader& entry)
     {
        entry.second->lock();
     }
   
   void SoundPlayer::callback(u8* buf, int len)
     {
        std::lock_guard<std::mutex> lock(_playListMutex);
        auto deleter = std::make_unique<mods::utils::BufferBackend::EmptyDeleter>();
        auto bufferBackend = std::make_shared<mods::utils::BufferBackend>(buf, len, std::move(deleter));
        mods::utils::RWBuffer<u8> rwbuf(bufferBackend);
        for(auto& entry : _playList) 
          {
             auto& reader = entry.first;
             if(!reader->isFinished())
               {
                  reader->read(&rwbuf, len);
                  sendProgress(*reader);
                  
                  if(reader->isFinished())
                    {
                       entry.second->unlock();
                    }
                  
                  return;
               }
          }
        std::cout << "TODO: SoundPlayer::callback() nothing to play, we should 0 volume" << std::endl;
     }
   
   void SoundPlayer::sendModuleInfo(const ModuleReader& module)
     {
        std::string info = module.getInfo();
        _moduleInfoCb(info);
     }
   
   void SoundPlayer::sendProgress(const ModuleReader& module)
     {
        std::string progress = module.getProgressInfo();
        _moduleProgressCb(progress);
     }
   
   SoundPlayer::SoundPlayerInitException::SoundPlayerInitException(std::string reason)
     : _reason(std::move(reason))
     {
     }
   
   const char* SoundPlayer::SoundPlayerInitException::what() const noexcept
     {
        return _reason.c_str();
     }

} // namespace mods
