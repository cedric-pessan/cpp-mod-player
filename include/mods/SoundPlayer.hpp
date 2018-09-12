#ifndef MODS_SOUNDPLAYER_HPP
#define MODS_SOUNDPLAYER_HPP

#include "ModuleReader.hpp"

#include <SDL.h>
#include <deque>
#include <exception>
#include <memory>
#include <mutex>
#include <string>

namespace mods
{
   class SoundPlayer
     {
      public:
        SoundPlayer();
        ~SoundPlayer();
        
        SoundPlayer(const SoundPlayer&) = delete;
        SoundPlayer(const SoundPlayer&&) = delete;
        SoundPlayer& operator=(const SoundPlayer&) = delete;
        SoundPlayer& operator=(const SoundPlayer&&) = delete;
        
        void play(ModuleReader::ptr reader);
        
      private:
        using SynchronizedReader = std::pair<ModuleReader::ptr, std::shared_ptr<std::mutex>>;
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
             SoundPlayerInitException(SoundPlayerInitException&&) = default;
             ~SoundPlayerInitException() override = default;
             const char* what() const noexcept override;
             
             SoundPlayerInitException() = delete;
             SoundPlayerInitException& operator=(const SoundPlayerInitException&) = delete;
             SoundPlayerInitException& operator=(const SoundPlayerInitException&&) = delete;
             
           private:
             std::string _reason;
          };
     };
   
} // namespace mods

#endif // MODS_SOUNDPLAYER_HPP