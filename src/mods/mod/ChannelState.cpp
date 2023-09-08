
#include "mods/mod/ChannelState.hpp"
#include "mods/mod/Note.hpp"

#include <iostream>
#include <limits>

namespace mods
{
   namespace mod
     {
        ChannelState::ChannelState(const std::vector<mods::utils::RBuffer<u8>>& sampleBuffers)
          : _currentValue(0.0, 0, false),
          _sampleBuffers(sampleBuffers)
            {
            }
        
        void ChannelState::prepareNextSample()
          {
             if(_currentValue.getLength() == 0)
               {
                  if(_instrument == 0)
                    {
                       _currentValue = RLESample(0.0, 0, false);
                       return;
                    }
                  auto& buffer = _sampleBuffers[_instrument-1];
                  if(_currentSample >= buffer.size())
                    {
                       _currentValue = RLESample(0.0, 0, false);
                       return;
                    }
                  u16 sample = buffer[_currentSample++];
                  auto convertedSample = toDouble(sample);
                  _currentValue = RLESample(convertedSample, _period, false);
               }
          }
        
        auto ChannelState::toDouble(s8 sample) -> double
          {
             double value = sample;
             if(value >= 0.0)
               {
                  value /= static_cast<double>(std::numeric_limits<s8>::max());
               }
             else
               {
                  value /= -static_cast<double>(std::numeric_limits<s8>::min());
               }
             return value;
          }
        
        auto ChannelState::getCurrentSampleLength() -> u32
          {
             return _currentValue.getLength();
          }
        
        auto ChannelState::readAndConsumeNextSample(u32 length) -> double
          {
             if(_currentValue.getLength() == 0)
               {
                  return 0.0;
               }
             _currentValue = RLESample(_currentValue.getValue(), _currentValue.getLength() - length, _currentValue.isFiltered());
             return _currentValue.getValue();
          }
        
        void ChannelState::updateChannelToNewLine(const mods::utils::RBuffer<Note>& note)
          {
             if(note->getInstrument() != 0)
               {
                  _instrument = note->getInstrument();
                  _currentSample = 0;
               }
             if(note->getPeriod() != 0)
               {
                  _period = note->getPeriod();
               }
          }
     } // namespace mod
} // namespace mod