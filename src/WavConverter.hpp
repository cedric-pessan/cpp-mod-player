#ifndef _WAVCONVERTER_HPP_
#define _WAVCONVERTER_HPP_

#include <memory>

namespace mods
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

#endif // _WAVCONVERTER_HPP_
