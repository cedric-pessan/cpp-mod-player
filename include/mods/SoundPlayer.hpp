#ifndef MODS_SOUNDPLAYER_HPP
#define MODS_SOUNDPLAYER_HPP

#include "ModuleReader.hpp"

#include <SDL.h>
#include <deque>
#include <exception>
#include <functional>
#include <memory>
#include <mutex>
#include <string>

namespace mods
{
   class SoundPlayer
     {
      public:
        using ModuleInfoCallback = std::function<void(const std::string&)>;
        using ModuleProgressCallback = std::function<void(const std::string&)>;
        
        SoundPlayer(ModuleInfoCallback moduleInfoCb, ModuleProgressCallback moduleProgressCb);
        ~SoundPlayer();
        
        SoundPlayer() = delete;
        SoundPlayer(const SoundPlayer&) = delete;
        SoundPlayer(SoundPlayer&&) = delete;
        auto operator=(const SoundPlayer&) -> SoundPlayer& = delete;
        auto operator=(SoundPlayer&&) -> SoundPlayer& = delete;
        
        void play(ModuleReader::ptr reader);
        
      private:
        static constexpr int _bufferSize = 1024;
        
        using SynchronizedReader = std::pair<ModuleReader::ptr, std::shared_ptr<std::mutex>>;
        std::mutex _playListMutex;
        std::deque<SynchronizedReader> _playList;
        ModuleInfoCallback _moduleInfoCb;
        ModuleProgressCallback _moduleProgressCb;
        
        static void checkInit(bool condition, const std::string& description);
        auto addReaderToPlayList(ModuleReader::ptr reader) -> const SynchronizedReader&;
        void removeOldestReaderFromPlayList();
        static void waitUntilFinished(const SynchronizedReader& entry);
        
        void sendModuleInfo(const ModuleReader& module);
        void sendProgress(const ModuleReader& module);
        
        static void s_ccallback(void* udata, Uint8* stream, int len);
        void callback(u8* buf, int len);
        
        class SoundPlayerInitException : public std::exception
          {
           public:
             explicit SoundPlayerInitException(std::string reason);
             SoundPlayerInitException(const SoundPlayerInitException&) noexcept = delete;
             SoundPlayerInitException(SoundPlayerInitException&&) = default;
             ~SoundPlayerInitException() override = default;
             auto what() const noexcept -> const char* override;
             
             SoundPlayerInitException() = delete;
             auto operator=(const SoundPlayerInitException&) -> SoundPlayerInitException& = delete;
             auto operator=(SoundPlayerInitException&&) -> SoundPlayerInitException& = delete;
             
           private:
             std::string _reason;
          };
     };
   
} // namespace mods

#endif // MODS_SOUNDPLAYER_HPP
