#ifndef _WAVREADER_HPP_
#define _WAVREADER_HPP_

#include "ModuleReader.hpp"

namespace mods
{
   
   class WavReader : public ModuleReader
     {
      public:
        WavReader();
        virtual ~WavReader();
        
        virtual bool isValid() const;
        
      private:
        WavReader(const WavReader&);
        WavReader& operator=(const WavReader&);
     };
   
}

#endif // _WAVREADER_HPP_