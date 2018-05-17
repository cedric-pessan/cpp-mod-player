#ifndef _SOUNDPLAYER_HPP_
#define _SOUNDPLAYER_HPP_

#include <exception>
#include <string>
#include <memory>

namespace mods
{
   class ModuleReader;
   
   class SoundPlayer
     {
      public:
        SoundPlayer();
        ~SoundPlayer();
        
        void play(const std::unique_ptr<ModuleReader>& reader);
        
      private:
        SoundPlayer(const SoundPlayer&);
        SoundPlayer& operator=(const SoundPlayer&);
        
        void check_init(bool condition, const std::string& description);
        void addReaderToPlayList();
        void removeOldestReaderFromPlayList();
        void waitUntilFinished();
        
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
