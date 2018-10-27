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
             
             StatCollector();
             
             StatCollector(const StatCollector&) = delete;
             StatCollector(const StatCollector&&) = delete;
             StatCollector& operator=(const StatCollector&) = delete;
             StatCollector& operator=(const StatCollector&&) = delete;
             ~StatCollector() = default;
             
             size_t getBytesRead() const;
             
             void inc(size_t i);
             
           private:
             size_t _value;
          };
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_STATCOLLECTOR_HPP
