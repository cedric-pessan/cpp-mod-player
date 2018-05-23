#ifndef _SOUNDPLAYER_HPP_
#define _SOUNDPLAYER_HPP_

#include "ModuleReader.hpp"

#include <exception>
#include <string>
#include <memory>
#include <mutex>
#include <deque>

namespace mods
{
   class SoundPlayer
     {
      public:
        SoundPlayer();
        ~SoundPlayer();
        
        void play(ModuleReader::ptr reader);
        
      private:
        SoundPlayer(const SoundPlayer&);
        SoundPlayer& operator=(const SoundPlayer&);
        
        typedef std::pair<ModuleReader::ptr, std::shared_ptr<std::mutex>> SynchronizedReader;
        std::mutex _playListMutex;
        std::deque<SynchronizedReader> _playList;
        
        void check_init(bool condition, const std::string& description);
        const SynchronizedReader& addReaderToPlayList(ModuleReader::ptr reader);
        void removeOldestReaderFromPlayList();
        void waitUntilFinished(const SynchronizedReader& entry);
        
        class SoundPlayerInitException : public std::exception
          {
           public:
             SoundPlayerInitException(const std::string& reason);
             SoundPlayerInitException(const SoundPlayerInitException&);
             virtual ~SoundPlayerInitException();
             virtual const char* what() const noexcept override;
             
           private:
             SoundPlayerInitException();
             SoundPlayerInitException& operator=(const SoundPlayerInitException&);
             
             std::string _reason;
          };
     };
   
}

#endif // _SOUNDPLAYER_HPP_
