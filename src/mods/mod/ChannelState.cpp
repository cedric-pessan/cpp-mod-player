
#include "mods/mod/Arpeggio.hpp"
#include "mods/mod/ChannelState.hpp"
#include "mods/mod/DelaySample.hpp"
#include "mods/mod/EffectType.hpp"
#include "mods/mod/Instrument.hpp"
#include "mods/mod/NoEffect.hpp"
#include "mods/mod/Note.hpp"
#include "mods/mod/Retrigger.hpp"
#include "mods/mod/SlideDown.hpp"
#include "mods/mod/SlideToNote.hpp"
#include "mods/mod/SlideToNoteAndVolumeSlide.hpp"
#include "mods/mod/SlideUp.hpp"
#include "mods/mod/Vibrato.hpp"
#include "mods/mod/VibratoAndVolumeSlide.hpp"
#include "mods/mod/VolumeSlide.hpp"
#include "mods/utils/AmigaRLESample.hpp"
#include "mods/utils/RBuffer.hpp"
#include "mods/utils/types.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <limits>
#include <memory>
#include <vector>

namespace mods
{
   namespace mod
     {
        namespace
          {
             constexpr double MAXIMUM_VOLUME = 64.0;
          } // namespace
        
        ChannelState::ChannelState(const std::vector<mods::utils::RBuffer<s8>>& sampleBuffers,
                                   const mods::utils::RBuffer<Instrument>& instruments)
          : _currentValue(0.0, static_cast<mods::utils::AmigaRLESample::SampleLength>(0), false),
          _sampleBuffers(&sampleBuffers),
          _instruments(instruments),
          _fineTuneFactors{}
            {
               using mods::utils::at;
               
               const double halfToneFactor = std::pow(2.0, -1.0 / (12.0 * 8.0));
               
               for(size_t i=0; i<_fineTuneFactors.size(); ++i)
                 {
                    const int fineTuneOffset = 8;
                    
                    auto fineTune = static_cast<double>(static_cast<int>(i) - fineTuneOffset);
                    
                    const double factor = std::pow(halfToneFactor, fineTune);
                    at(_fineTuneFactors, i) = factor;
                 }
               
               _noEffect = std::make_unique<NoEffect>();
               _vibrato = std::make_unique<Vibrato>();
               _volumeSlide = std::make_unique<VolumeSlide>();
               _vibratoAndVolumeSlide = std::make_unique<VibratoAndVolumeSlide>(_vibrato.get(), _volumeSlide.get());
               _arpeggio = std::make_unique<Arpeggio>();
               _slideDown = std::make_unique<SlideDown>();
               _slideUp = std::make_unique<SlideUp>();
               _slideToNote = std::make_unique<SlideToNote>();
               _slideToNoteAndVolumeSlide = std::make_unique<SlideToNoteAndVolumeSlide>(_slideToNote.get(), _volumeSlide.get());
               _retrigger = std::make_unique<Retrigger>();
               _delaySample = std::make_unique<DelaySample>();
               
               _currentEffect = _noEffect.get();
            }
        
        void ChannelState::prepareNextSample()
          {
             using mods::utils::AmigaRLESample;
             
             if(_currentValue.getLength() == 0)
               {
                  if(_instrument == 0)
                    {
                       _currentValue = RLESample(0.0, static_cast<AmigaRLESample::SampleLength>(0), _filtered);
                       return;
                    }
                  if(_currentEffect->retriggerSample())
                    {
                       _currentSample = 0;
                       _currentRepeatSample = 0;
                    }
                  const auto& buffer = (*_sampleBuffers)[_instrument-1];
                  if(_currentSample >= buffer.size())
                    {
                       const auto& instrument = _instruments[_instrument-1];
                       
                       if(instrument.getRepeatLength() > 0 && _currentEffect->isSampleEnabled())
                         {
                            if(_currentRepeatSample == 0 ||
                               _currentRepeatSample >= instrument.getRepeatOffset() + instrument.getRepeatLength())
                              {
                                 _currentRepeatSample = instrument.getRepeatOffset();
                              }
                            const s8 sample = buffer[_currentRepeatSample++];
                            processNextSample(sample);
                         }
                       else
                         {
                            _currentValue = RLESample(0.0, static_cast<AmigaRLESample::SampleLength>(0), _filtered);
                         }
                       return;
                    }
                  const s8 sample = buffer[_currentSample++];
                  processNextSample(sample);
               }
          }
        
        void ChannelState::processNextSample(s8 sample)
          {
             using mods::utils::AmigaRLESample;
             
             _period = _currentEffect->getModifiedPeriod(_period);
             
             auto convertedSample = toDouble(sample);
             
             _currentValue = RLESample(convertedSample, static_cast<AmigaRLESample::SampleLength>(_period), _filtered);
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
             using mods::utils::AmigaRLESample;
             
             if(_currentValue.getLength() == 0)
               {
                  return 0.0;
               }
             _currentValue = RLESample(_currentValue.getValue(), static_cast<AmigaRLESample::SampleLength>(_currentValue.getLength() - length), _currentValue.isFiltered());
             _volume = _currentEffect->getModifiedVolume(_volume);
             return _currentValue.getValue() * static_cast<double>(_volume) / MAXIMUM_VOLUME;
          }
        
        auto ChannelState::getFineTuneFactor(int fineTune) -> double
          {
             constexpr int OFFSET_FINETUNE_0 = 8;
             
             using mods::utils::at;
             return at(_fineTuneFactors, fineTune + OFFSET_FINETUNE_0);
          }
        
        void ChannelState::updateChannelToNewLine(const mods::utils::RBuffer<Note>& note)
          {
             using mods::utils::AmigaRLESample;
             
             _speedSetOnLastLine = false;
             _hasPatternJump = false;
             _startOfLoop = false;
             _endOfLoop = false;
             _hasNewFilterValue = false;
             
             if(note->getEffect() != EffectType::SLIDE_TO_NOTE)
               {
                  if(note->getInstrument() != 0)
                    {
                       _instrument = note->getInstrument();
                       _volume = _instruments[_instrument-1].getVolume();
                       _currentSample = 0;
                       _currentRepeatSample = 0;
                       _currentValue = RLESample(0.0, static_cast<AmigaRLESample::SampleLength>(0), _filtered);
                    }
                  if(note->getPeriod() != 0)
                    {
                       _currentSample = 0;
                       _currentRepeatSample = 0;
                       _period = note->getPeriod();
                       _currentValue = RLESample(0.0, static_cast<AmigaRLESample::SampleLength>(0), _filtered);
                       
                       if(_instrument != 0)
                         {
                            if(_instruments[_instrument-1].getFineTune() != 0)
                              {
                                 auto fineTune = _instruments[_instrument-1].getFineTune();
                                 auto factor = getFineTuneFactor(fineTune);
                                 _period = static_cast<u16>(std::round(static_cast<double>(_period) * factor));
                              }
                         }
                    }
               }
             
             applyEffect(note);
          }
        
        void ChannelState::applyEffect(const mods::utils::RBuffer<Note>& note)
          {
             const auto effect = note->getEffect();
             switch(effect)
               {
                case EffectType::ARPEGGIO:
                  applyArpeggioEffect(note);
                  break;
                  
                case EffectType::SLIDE_UP:
                  applySlideUpEffect(note);
                  break;
                  
                case EffectType::SLIDE_DOWN:
                  applySlideDownEffect(note);
                  break;
                  
                case EffectType::SLIDE_TO_NOTE:
                  applySlideToNoteEffect(note);
                  break;
                  
                case EffectType::VIBRATO:
                  applyVibratoEffect(note);
                  break;
                  
                case EffectType::SLIDE_TO_NOTE_AND_VOLUME_SLIDE:
                  applySlideToNoteAndVolumeSlideEffect(note);
                  break;
                  
                case EffectType::VIBRATO_AND_VOLUME_SLIDE:
                  applyVibratoAndVolumeSlideEffect(note);
                  break;
                  
                case EffectType::SET_SAMPLE_OFFSET:
                  applySetSampleOffsetEffect(note);
                  break;
                  
                case EffectType::VOLUME_SLIDE:
                  applyVolumeSlideEffect(note);
                  break;
                  
                case EffectType::POSITION_JUMP:
                  applyPositionJumpEffect(note);
                  break;
                  
                case EffectType::SET_VOLUME:
                  applySetVolumeEffect(note);
                  break;
                  
                case EffectType::PATTERN_BREAK:
                  applyPatternBreakEffect(note);
                  break;
                  
                case EffectType::EXTENDED_EFFECT:
                  applyExtendedEffect(note);
                  break;
                  
                case EffectType::SET_SPEED:
                  applySetSpeedEffect(note);
                  break;
                  
                default:
                  std::cout << "unknown effect:" << std::hex << static_cast<u32>(toUnderlying(effect)) << std::dec << '\n';
               }
          }
        
        void ChannelState::applyArpeggioEffect(const mods::utils::RBuffer<Note>& note)
          {
             const u32 arg = note->getEffectArgument();
             if(arg != 0)
               {
                  const u16 argX = arg >> 4U;
                  const u16 argY = arg & 0xFU;
                  _arpeggio->init(Arpeggio::Parameters{argX, argY, _period});
                  _currentEffect = _arpeggio.get();
               }
             else
               {
                  _currentEffect = _noEffect.get();
               }
          }
        
        void ChannelState::applySlideUpEffect(const mods::utils::RBuffer<Note>& note)
          {
             const auto arg = static_cast<int>(note->getEffectArgument());
             _slideUp->init(arg);
             _currentEffect = _slideUp.get();
          }
        
        void ChannelState::applySlideDownEffect(const mods::utils::RBuffer<Note>& note)
          {
             const auto arg = static_cast<int>(note->getEffectArgument());
             _slideDown->init(arg);
             _currentEffect = _slideDown.get();
          }
        
        void ChannelState::applySlideToNoteEffect(const mods::utils::RBuffer<Note>& note)
          {
             const auto arg = static_cast<int>(note->getEffectArgument());
             auto targetPeriod = note->getPeriod();
             auto fineTune = _instruments[_instrument-1].getFineTune();
             auto factor = getFineTuneFactor(fineTune);
             targetPeriod = static_cast<u16>(std::round(static_cast<double>(targetPeriod) * factor));
             _slideToNote->init(static_cast<SlideToNote::Period>(targetPeriod), arg);
             _currentEffect = _slideToNote.get();
          }
        
        void ChannelState::applyVibratoEffect(const mods::utils::RBuffer<Note>& note)
          {
             const u32 arg = note->getEffectArgument();
             const u32 oscillationFrequency = arg >> 4U;
             const u32 amplitude = arg & 0xFU;
             if(arg != 0)
               {
                  _vibrato->init(static_cast<Vibrato::Depth>(amplitude), static_cast<Vibrato::VibratoFrequency>(oscillationFrequency), _period);
               }
             else
               {
                  _vibrato->tick();
               }
             _currentEffect = _vibrato.get();
          }
        
        void ChannelState::applySlideToNoteAndVolumeSlideEffect(const mods::utils::RBuffer<Note>& note)
          {
             using Volume = SlideToNoteAndVolumeSlide::Volume;
             using Delta = SlideToNoteAndVolumeSlide::Delta;
             
             const u32 arg = note->getEffectArgument();
             const u8 slideUp = (arg >> 4U) & 0xFU;
             const u8 slideDown = arg & 0xFU;
             if(slideUp > 0 && slideDown > 0)
               {
                  _slideToNoteAndVolumeSlide->init(static_cast<Volume>(_volume), static_cast<Delta>(0));
               }
             else if(slideUp > 0)
               {
                  _slideToNoteAndVolumeSlide->init(static_cast<Volume>(_volume), static_cast<Delta>(slideUp));
               }
             else
               {
                  _slideToNoteAndVolumeSlide->init(static_cast<Volume>(_volume), static_cast<Delta>(-slideDown));
               }
             _currentEffect = _slideToNoteAndVolumeSlide.get();
          }
        
        void ChannelState::applyVibratoAndVolumeSlideEffect(const mods::utils::RBuffer<Note>& note)
          {
             const u32 arg = note->getEffectArgument();
             const u8 slideUp = (arg >> 4U) & 0xFU;
             const u8 slideDown = arg & 0xFU;
             _vibrato->tick();
             if(slideUp > 0 && slideDown > 0)
               {
                  _vibratoAndVolumeSlide->init(_volume, 0);
               }
             else if(slideUp > 0)
               {
                  _vibratoAndVolumeSlide->init(_volume, slideUp);
               }
             else
               {
                  _vibratoAndVolumeSlide->init(_volume, static_cast<s16>(-slideDown));
               }
             _currentEffect = _vibratoAndVolumeSlide.get();
          }
        
        void ChannelState::applySetSampleOffsetEffect(const mods::utils::RBuffer<Note>& note)
          {
             const u32 arg = note->getEffectArgument();
             if(arg != 0)
               {
                  _currentSample = arg << BITS_IN_BYTE;
                  _currentRepeatSample = 0;
               }
             _currentEffect = _noEffect.get();
          }
        
        void ChannelState::applyVolumeSlideEffect(const mods::utils::RBuffer<Note>& note)
          {
             using Volume = VolumeSlide::Volume;
             using Delta = VolumeSlide::Delta;
             
             const u32 arg = note->getEffectArgument();
             const u8 slideUp = (arg >> 4U) & 0xFU;
             const u8 slideDown = arg & 0xFU;
             if(slideUp > 0 && slideDown > 0)
               {
                  _volumeSlide->init(static_cast<Volume>(_volume), static_cast<Delta>(0));
               }
             else if(slideUp > 0)
               {
                  _volumeSlide->init(static_cast<Volume>(_volume), static_cast<Delta>(slideUp));
               }
             else
               {
                  _volumeSlide->init(static_cast<Volume>(_volume), static_cast<Delta>(-slideDown));
               }
             _currentEffect = _volumeSlide.get();
          }
        
        void ChannelState::applyPositionJumpEffect(const mods::utils::RBuffer<Note>& note)
          {
             _hasPatternJump = true;
             _patternOfJumpTarget = static_cast<int>(note->getEffectArgument());
             _lineOfJumpTarget = 0;
          }
        
        void ChannelState::applySetVolumeEffect(const mods::utils::RBuffer<Note>& note)
          {
             _volume = std::min(note->getEffectArgument(), static_cast<u32>(MAXIMUM_VOLUME));
             _currentEffect = _noEffect.get();
          }
        
        void ChannelState::applyPatternBreakEffect(const mods::utils::RBuffer<Note>& note)
          {
             constexpr static int xCoef = 10;
             constexpr static int linesPerPattern = 64;
             
             const u32 arg = note->getEffectArgument();
             const u8 xArg = (arg >> 4U) & 0xFU;
             const u8 yArg = arg & 0xFU;
             _hasPatternJump = true;
             _patternOfJumpTarget = -1;
             _lineOfJumpTarget = xArg * xCoef + yArg;
             if(_lineOfJumpTarget >= linesPerPattern) 
               {
                  _lineOfJumpTarget = 0;
               }
          }
        
        void ChannelState::applyExtendedEffect(const mods::utils::RBuffer<Note>& note)
          {
             const auto extendedEffect = note->getExtendedEffect();
             switch(extendedEffect)
               {
                case ExtendedEffectType::SET_FILTER:
                  applySetFilter(note);
                  break;
                  
                case ExtendedEffectType::FINE_SLIDE_UP:
                  applyFineSlideUpEffect(note);
                  break;
                  
                case ExtendedEffectType::PATTERN_LOOP:
                  applyPatternLoopEffect(note);
                  break;
                  
                case ExtendedEffectType::RETRIGGER_SAMPLE:
                  applyRetriggerSampleEffect(note);
                  break;
                  
                case ExtendedEffectType::DELAY_SAMPLE:
                  applyDelaySampleEffect(note);
                  break;
                  
                default:
                  std::cout << "unknown extended effect:" << std::hex << static_cast<u32>(toUnderlying(extendedEffect)) << std::dec << '\n';
               }
          }
        
        void ChannelState::applySetSpeedEffect(const mods::utils::RBuffer<Note>& note)
          {
             _speedSetOnLastLine = true;
             _speed = note->getEffectArgument();
             _currentEffect = _noEffect.get();
          }
        
        void ChannelState::applySetFilter(const mods::utils::RBuffer<Note>& note)
          {
             _hasNewFilterValue = true;
             _newFilterValue = (note->getExtendedEffectArgument() & 1U) == 0;
          }
        
        void ChannelState::applyFineSlideUpEffect(const mods::utils::RBuffer<Note>& note)
          {
             _period -= note->getExtendedEffectArgument();
             _currentEffect = _noEffect.get();
          }
        
        void ChannelState::applyPatternLoopEffect(const mods::utils::RBuffer<Note>& note)
          {
             _loopLength = note->getExtendedEffectArgument();
             if(_loopLength == 0)
               {
                  _startOfLoop = true;
               }
             else
               {
                  _endOfLoop = true;
               }
             _currentEffect = _noEffect.get();
          }
        
        void ChannelState::applyRetriggerSampleEffect(const mods::utils::RBuffer<Note>& note)
          {
             const u32 retriggerPeriod = note->getExtendedEffectArgument();
             
             _retrigger->init(retriggerPeriod);
             _currentEffect = _retrigger.get();
          }
        
        void ChannelState::applyDelaySampleEffect(const mods::utils::RBuffer<Note>& note)
          {
             if(note->getInstrument() != 0 && note->getPeriod() != 0)
               {
                  const auto& buffer = (*_sampleBuffers)[_instrument-1];
                  _currentSample = buffer.size();
                  _delaySample->init(note->getExtendedEffectArgument());
                  _currentEffect = _delaySample.get();
               }
          }
        
        auto ChannelState::hasSpeedDefined() const -> bool
          {
             return _speedSetOnLastLine;
          }
        
        auto ChannelState::getSpeed() const -> u32
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
        
        auto ChannelState::isStartOfLoop() const -> bool
          {
             return _startOfLoop;
          }
        
        auto ChannelState::isEndOfLoop() const -> bool
          {
             return _endOfLoop;
          }
        
        auto ChannelState::getLoopLength() const -> u32
          {
             return _loopLength;
          }
        
        auto ChannelState::hasNewFilterValue() const -> bool
          {
             return _hasNewFilterValue;
          }
        
        auto ChannelState::getNewFilterValue() const -> bool
          {
             return _newFilterValue;
          }
        
        void ChannelState::setFilterValue(bool filtered)
          {
             _filtered = filtered;
          }
        
        void ChannelState::tick()
          {
             _currentEffect->tick();
          }
     } // namespace mod
} // namespace mods
