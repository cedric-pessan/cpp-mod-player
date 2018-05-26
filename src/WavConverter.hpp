#ifndef _WAVCONVERTER_HPP_
#define _WAVCONVERTER_HPP_

#include <memory>

namespace mods
{
   class WavConverter
     {
      public:
        typedef std::unique_ptr<WavConverter> ptr;
        
        WavConverter();
        virtual ~WavConverter();
        
        bool isFinished() const;
        
      private:
        WavConverter(const WavConverter&);
        WavConverter& operator=(const WavConverter&);
     };
}

#endif // _WAVCONVERTER_HPP_
