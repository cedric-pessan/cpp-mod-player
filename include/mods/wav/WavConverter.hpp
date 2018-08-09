#ifndef WAVCONVERTER_HPP
#define WAVCONVERTER_HPP

#include <memory>

namespace mods
{
   namespace wav
     {
        class WavConverter
          {
           public:
             typedef std::unique_ptr<WavConverter> ptr;
             
             static ptr buildConverter(int bitsPerSample);
             
             WavConverter() = default;
             virtual ~WavConverter() = default;
             
             virtual bool isFinished() const = 0;
             
           private:
             WavConverter(const WavConverter&) = delete;
             WavConverter& operator=(const WavConverter&) = delete;
          };
     }
}

#endif // WAVCONVERTER_HPP
