#ifndef _SOUNDPLAYER_HPP_
#define _SOUNDPLAYER_HPP_

#include <exception>
#include <string>
#include <memory>
#include <mutex>
#include <deque>

namespace mods
{
   class ModuleReader;
   
   class SoundPlayer
     {
      public:
        SoundPlayer();
        ~SoundPlayer();
        
        void play(std::unique_ptr<ModuleReader> reader);
        
      private:
        SoundPlayer(const SoundPlayer&);
        SoundPlayer& operator=(const SoundPlayer&);
        
        void check_init(bool condition, const std::string& description);
        std::shared_ptr<std::mutex> addReaderToPlayList(std::unique_ptr<ModuleReader> reader);
        void removeOldestReaderFromPlayList();
        void waitUntilFinished(const std::shared_ptr<std::mutex>& entryMutex);
        
        typedef std::pair<std::unique_ptr<ModuleReader>, std::shared_ptr<std::mutex>> SynchronizedReader;
        std::mutex _playListMutex;
        std::deque<SynchronizedReader> _playList;
        
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
