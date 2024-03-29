
#include "mods/ModuleReader.hpp"
#include "mods/SoundPlayer.hpp"
#include "mods/StandardFrequency.hpp"

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
             auto* sp = static_cast<SoundPlayer*>(udata);
             sp->callback(stream, len);
          }
     }
   
   SoundPlayer::SoundPlayer(ModuleInfoCallback moduleInfoCb, ModuleProgressCallback moduleProgressCb)
     : _moduleInfoCb(std::move(moduleInfoCb)),
     _moduleProgressCb(std::move(moduleProgressCb))
       {
          int res = SDL_Init(SDL_INIT_AUDIO);
          checkInit(res == 0, "sdl audio subsystem could not be initialized");
          SDL_AudioSpec spec;
          spec.freq = toUnderlying(StandardFrequency::_44100);
          spec.format = AUDIO_S16;
          spec.channels = 2;
          spec.samples = _bufferSize;
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
   
   void SoundPlayer::checkInit(bool condition, const std::string& description)
     {
        if(!condition)
          {
             throw SoundPlayerInitException(description);
          }
     }
   
   void SoundPlayer::play(ModuleReader::ptr reader)
     {
        sendModuleInfo(*reader);
        const auto& entry = addReaderToPlayList(std::move(reader));
        SDL_PauseAudio(0);
        waitUntilFinished(entry);
        SDL_PauseAudio(1);
        removeOldestReaderFromPlayList();
     }
   
   auto SoundPlayer::addReaderToPlayList(ModuleReader::ptr reader) -> const SoundPlayer::SynchronizedReader&
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
        assert((len % 2) == 0);
        std::lock_guard<std::mutex> lock(_playListMutex);
        auto deleter = std::make_unique<mods::utils::RWBufferBackend::EmptyDeleter>();
        auto bufferBackend = std::make_unique<mods::utils::RWBufferBackend>(buf, len, std::move(deleter));
        mods::utils::RWBuffer<s16> rwbuf(std::move(bufferBackend));
        for(auto& entry : _playList) 
          {
             auto& reader = entry.first;
             if(!reader->isFinished())
               {
                  reader->read(&rwbuf);
                  sendProgress(*reader);
                  
                  if(reader->isFinished())
                    {
                       entry.second->unlock();
                    }
                  
                  return;
               }
          }
        for(auto& elem : rwbuf)
          {
             elem = 0;
          }
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
   
   auto SoundPlayer::SoundPlayerInitException::what() const noexcept -> const char*
     {
        return _reason.c_str();
     }

} // namespace mods
