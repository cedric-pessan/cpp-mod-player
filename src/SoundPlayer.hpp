#ifndef _SOUNDPLAYER_HPP_
#define _SOUNDPLAYER_HPP_

#include "ModuleReader.hpp"

#include <exception>
#include <string>
#include <memory>
#include <mutex>
#include <deque>
#include <SDL.h>

namespace mods
{
   class SoundPlayer
     {
      public:
        SoundPlayer();
        ~SoundPlayer();
        
        void play(ModuleReader::ptr reader);
        
      private:
        SoundPlayer(const SoundPlayer&) = delete;
        SoundPlayer& operator=(const SoundPlayer&) = delete;
        
        typedef std::pair<ModuleReader::ptr, std::shared_ptr<std::mutex>> SynchronizedReader;
        std::mutex _playListMutex;
        std::deque<SynchronizedReader> _playList;
        
        void checkInit(bool condition, const std::string& description) const;
        const SynchronizedReader& addReaderToPlayList(ModuleReader::ptr reader);
        void removeOldestReaderFromPlayList();
        void waitUntilFinished(const SynchronizedReader& entry);
        
        static void s_ccallback(void* udata, Uint8* stream, int len);
        void callback();
        
        class SoundPlayerInitException : public std::exception
          {
           public:
             explicit SoundPlayerInitException(std::string reason);
             SoundPlayerInitException(const SoundPlayerInitException&) noexcept;
             virtual ~SoundPlayerInitException() = default;
             virtual const char* what() const noexcept override;
             
           private:
             SoundPlayerInitException() = delete;
             SoundPlayerInitException& operator=(const SoundPlayerInitException&) = delete;
             
             std::string _reason;
          };
     };
   
}

#endif // _SOUNDPLAYER_HPP_
