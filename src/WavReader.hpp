#ifndef _WAVREADER_HPP_
#define _WAVREADER_HPP_

#include "ModuleReader.hpp"
#include "WavConverter.hpp"

namespace mods
{
   
   class WavReader : public ModuleReader
     {
      public:
        WavReader();
        virtual ~WavReader();
        
        virtual bool isFinished() const override;
        
      private:
        WavReader(const WavReader&);
        WavReader& operator=(const WavReader&);
        
        WavConverter::ptr _converter;
     };
   
}

#endif // _WAVREADER_HPP_