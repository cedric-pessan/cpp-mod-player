
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
        ChannelState::ChannelState(const std::vector<mods::utils::RBuffer<s8>>& sampleBuffers,
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
               
               _noEffect = std::make_unique<NoEffect>();
               _vibrato = std::make_unique<Vibrato>();
               _volumeSlide = std::make_unique<VolumeSlide>();
               _vibratoAndVolumeSlide = std::make_unique<VibratoAndVolumeSlide>(_vibrato.get(), _volumeSlide.get());
               _arpeggio = std::make_unique<Arpeggio>();
               _slideDown = std::make_unique<SlideDown>();
               _slideUp = std::make_unique<SlideUp>();
               
               _currentEffect = _noEffect.get();
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
                            s8 sample = buffer[_currentRepeatSample++];
                            processNextSample(sample);
                         }
                       else
                         {
                            _currentValue = RLESample(0.0, 0, false);
                         }
                       return;
                    }
                  s8 sample = buffer[_currentSample++];
                  processNextSample(sample);
               }
          }
        
        void ChannelState::processNextSample(s8 sample)
          {
             _period = _currentEffect->getModifiedPeriod(_period);
             
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
             _volume = _currentEffect->getModifiedVolume(_volume);
             return _currentValue.getValue() * static_cast<double>(_volume) / 64.0;
          }
        
        auto ChannelState::getFineTuneFactor(int fineTune) -> double
          {
             using mods::utils::at;
             return at(_fineTuneFactors, fineTune + 8);
          }
        
        void ChannelState::updateChannelToNewLine(const mods::utils::RBuffer<Note>& note)
          {
             _speedSetOnLastLine = false;
             _hasPatternJump = false;
             
             if(note->getInstrument() != 0)
               {
                  _instrument = note->getInstrument();
                  _volume = _instruments[_instrument-1].getVolume();
                  _currentSample = 0;
                  _currentRepeatSample = 0;
                  _currentValue = RLESample(0.0, 0, false);
               }
             if(note->getPeriod() != 0)
               {
                  _currentSample = 0;
                  _currentRepeatSample = 0;
                  _period = note->getPeriod();
                  _currentValue = RLESample(0.0, 0, false);
                  
                  if(_instrument != 0)
                    {
                       if(_instruments[_instrument-1].getFineTune() != 0)
                         {
                            auto fineTune = _instruments[_instrument-1].getFineTune();
                            auto factor = getFineTuneFactor(fineTune);
                            _period = std::round(static_cast<double>(_period) * factor);
                         }
                    }
               }
             
             u32 effect = note->getEffect();
             switch(effect)
               {
                case 0x0: // arpeggio
                    {
                       u32 arg = note->getEffectArgument();
                       if(arg != 0)
                         {
                            u32 x = arg >> 4;
                            u32 y = arg & 0xF;
                            _arpeggio->init(x, y, _period);
                            _currentEffect = _arpeggio.get();
                         }
                       else
                         {
                            _currentEffect = _noEffect.get();
                         }
                    }
                  break;
                  
                case 0x1: // slide up
                    {
                       u32 arg = note->getEffectArgument();
                       _slideUp->init(arg);
                       _currentEffect = _slideUp.get();
                    }
                  break;
                  
                case 0x2: // slide down
                    {
                       u32 arg = note->getEffectArgument();
                       _slideDown->init(arg);
                       _currentEffect = _slideDown.get();
                    }
                  break;
                  
                case 0x4: // vibrato
                    {
                       u32 arg = note->getEffectArgument();
                       u32 oscillationFrequency = arg >> 4;
                       u32 amplitude = arg & 0xF;
                       if(arg != 0)
                         {
                            _vibrato->init(amplitude, oscillationFrequency, _period);
                         }
                       else
                         {
                            _vibrato->tick();
                         }
                       _currentEffect = _vibrato.get();
                    }
                  break;
                  
                case 0x6: // vibrato + volume slide
                    {
                       u32 arg = note->getEffectArgument();
                       u8 slideUp = (arg >> 4) & 0xF;
                       u8 slideDown = arg & 0xF;
                       _vibrato->tick();
                       _vibratoAndVolumeSlide->init(_volume, slideUp, slideDown);
                       _currentEffect = _vibratoAndVolumeSlide.get();
                    }
                  break;
                  
                case 0x9: // set sample offset
                    {
                       u32 arg = note->getEffectArgument();
                       if(arg)
                         {
                            _currentSample = arg << 8;
                            _currentRepeatSample = 0;
                         }
                       _currentEffect = _noEffect.get();
                    }
                  break;
                  
                case 0xa: // volume slide
                    {
                       u32 arg = note->getEffectArgument();
                       u8 slideUp = (arg >> 4) & 0xF;
                       u8 slideDown = arg & 0xF;
                       _volumeSlide->init(_volume, slideUp, slideDown);
                       _currentEffect = _volumeSlide.get();
                    }
                  break;
                  
                case 0xc: // set volume
                  _volume = note->getEffectArgument();
                  if(_volume > 64)
                    {
                       _volume = 64;
                    }
                  _currentEffect = _noEffect.get();
                  break;
                  
                case 0xd: // pattern break
                    {
                       u32 arg = note->getEffectArgument();
                       u8 x = (arg >> 4) & 0xF;
                       u8 y = arg & 0xF;
                       _hasPatternJump = true;
                       _patternOfJumpTarget = -1;
                       _lineOfJumpTarget = x * 10 + y;
                       if(_lineOfJumpTarget > 63) 
                         {
                            _lineOfJumpTarget = 0;
                         }
                    }
                  break;
                  
                case 0xf: // set speed
                  _speedSetOnLastLine = true;
                  _speed = note->getEffectArgument();
                  _currentEffect = _noEffect.get();
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
        
        auto ChannelState::hasPatternJump() const -> bool
          {
             return _hasPatternJump;
          }
        
        auto ChannelState::getPatternOfJumpTarget() const -> int
          {
             return _patternOfJumpTarget;
          }
        
        auto ChannelState::getLineOfJumpTarget() const -> int
          {
             return _lineOfJumpTarget;
          }
        
        void ChannelState::tick()
          {
             _currentEffect->tick();
          }
     } // namespace mod
} // namespace mod
