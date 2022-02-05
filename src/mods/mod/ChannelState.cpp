
#include "mods/mod/ChannelState.hpp"
#include "mods/mod/Note.hpp"

#include <iostream>

namespace mods
{
   namespace mod
     {
        ChannelState::ChannelState(const std::vector<mods::utils::RBuffer<u8>>& sampleBuffers)
          : _sampleBuffers(sampleBuffers)
            {
            }
        
        auto ChannelState::readNextSample() -> s16
          {
             if(_instrument == 0)
               {
                  return 0;
               }
             auto& buffer = _sampleBuffers[_instrument-1];
             if(_currentSample >= buffer.size())
               {
                  return 0;
               }
             u16 sample = buffer[_currentSample++];
             sample = (sample << 8) | ((sample << 1) & 0xFF);
             return static_cast<s16>(sample);
          }
        
        void ChannelState::updateChannelToNewLine(const mods::utils::RBuffer<Note>& note)
          {
             _instrument = note->getInstrument();
             _currentSample = 0;
          }
     } // namespace mod
} // namespace mod