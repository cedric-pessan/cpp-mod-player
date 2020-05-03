#ifndef MODS_WAV_STATCOLLECTOR_HPP
#define MODS_WAV_STATCOLLECTOR_HPP

#include <memory>

namespace mods
{
   namespace wav
     {
        class StatCollector
          {
           public:
             using sptr = std::shared_ptr<StatCollector>;
             
             StatCollector() = default;
             
             StatCollector(const StatCollector&) = delete;
             StatCollector(StatCollector&&) = delete;
             auto operator=(const StatCollector&) -> StatCollector& = delete;
             auto operator=(StatCollector&&) -> StatCollector& = delete;
             ~StatCollector() = default;
             
             auto getBytesRead() const -> size_t;
             
             void inc(size_t i);
             
           private:
             size_t _value = 0;
          };
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_STATCOLLECTOR_HPP
