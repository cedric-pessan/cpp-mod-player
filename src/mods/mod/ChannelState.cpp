
#include "mods/mod/ChannelState.hpp"
#include "mods/mod/Instrument.hpp"
#include "mods/mod/Note.hpp"

#include <cmath>
#include <iostream>
#include <limits>

namespace mods
{
   namespace mod
     {
        ChannelState::ChannelState(const std::vector<mods::utils::RBuffer<u8>>& sampleBuffers,
                                   const mods::utils::RBuffer<Instrument>& instruments)
          : _currentValue(0.0, 0, false),
          _sampleBuffers(sampleBuffers),
          _instruments(instruments)
            {
               using mods::utils::at;
               
               double halfToneFactor = std::pow(2.0, -1.0 / (12.0 * 8.0));
               
               for(int i=0; i<16; ++i)
                 {
                    double fineTune = static_cast<double>(i-8);
                    
                    double factor = std::pow(halfToneFactor, fineTune);
                    at(_fineTuneFactors, i) = factor;
                 }
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
                       auto& instrument = _instruments[_instrument-1];
                       if(instrument.getRepeatLength() > 0)
                         {
                            if(_currentRepeatSample == 0 ||
                               _currentRepeatSample >= instrument.getRepeatOffset() + instrument.getRepeatLength())
                              {
                                 _currentRepeatSample = instrument.getRepeatOffset();
                              }
                            u16 sample = buffer[_currentRepeatSample++];
                            processNextSample(sample);
                         }
                       else
                         {
                            _currentValue = RLESample(0.0, 0, false);
                         }
                       return;
                    }
                  u16 sample = buffer[_currentSample++];
                  processNextSample(sample);
               }
          }
        
        void ChannelState::processNextSample(u16 sample)
          {
             sample = sample * _volume / 64;
             auto convertedSample = toDouble(sample);
             _currentValue = RLESample(convertedSample, _period, false);
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
        
        auto ChannelState::getFineTuneFactor(int fineTune) -> double
          {
             using mods::utils::at;
             return at(_fineTuneFactors, fineTune + 8);
          }
        
        void ChannelState::updateChannelToNewLine(const mods::utils::RBuffer<Note>& note)
          {
             _speedSetOnLastLine = false;
             
             if(note->getInstrument() != 0)
               {
                  _instrument = note->getInstrument();
                  _currentSample = 0;
               }
             if(note->getPeriod() != 0)
               {
                  _period = note->getPeriod();
               }
             if(_instruments[_instrument-1].getFineTune() != 0)
               {
                  auto fineTune = _instruments[_instrument-1].getFineTune();
                  auto factor = getFineTuneFactor(fineTune);;
                  _period = std::round(static_cast<double>(_period) * factor);
               }
             _volume = _instruments[_instrument-1].getVolume();
             
             u32 effect = note->getEffect();
             switch(effect)
               {
                case 0xf:
                  _speedSetOnLastLine = true;
                  _speed = note->getEffectArgument();
                  break;
                  
                default:
                  std::cout << "unknown effect:" << std::hex << effect << std::dec << std::endl;
               }
          }
        
        auto ChannelState::hasSpeedDefined() const -> bool
          {
             return _speedSetOnLastLine;
          }
        
        auto ChannelState::getSpeed() const -> int
          {
             return _speed;
          }
     } // namespace mod
} // namespace mod
