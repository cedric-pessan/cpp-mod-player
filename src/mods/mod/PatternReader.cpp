
#include "mods/StandardFrequency.hpp"
#include "mods/mod/ChannelId.hpp"
#include "mods/mod/ChannelState.hpp"
#include "mods/mod/Instrument.hpp"
#include "mods/mod/Note.hpp"
#include "mods/mod/PatternReader.hpp"
#include "mods/utils/AmigaRLESample.hpp"
#include "mods/utils/RBuffer.hpp"
#include "mods/utils/types.hpp"

#include <array>
#include <cstddef>
#include <vector>

namespace mods
{
   namespace mod
     {
        namespace
          {
             enum struct ChannelLocation : u8
               {
                  LEFT,
                    RIGHT
               };
             
             constexpr size_t channelGroupSize = 4;
             
             constexpr std::array<ChannelLocation, channelGroupSize> channelLocationInGroup
               {
                  ChannelLocation::LEFT, 
                    ChannelLocation::RIGHT, 
                    ChannelLocation::RIGHT,
                    ChannelLocation::LEFT
               };
          } // namespace
        
        PatternReader::PatternReader(size_t nbChannels,
                                     const mods::utils::RBuffer<Note>& patternBuffer,
                                     const mods::utils::RBuffer<Instrument>& instruments,
                                     const std::vector<mods::utils::RBuffer<s8>>& sampleBuffers,
                                     std::array<OutputQueue, 2>* outputs)
          : _patternBuffer(patternBuffer),
          _leftOutput(&outputs->at(toUnderlying(ChannelId::LEFT))),
          _rightOutput(&outputs->at(toUnderlying(ChannelId::RIGHT)))
            {
               using mods::utils::at;
               
               for(size_t i=0; i<nbChannels; ++i)
                 {
                    _channels.emplace_back(sampleBuffers, instruments);
                 }
               for(size_t i=0; i<nbChannels; ++i)
                 {
                    if(at(channelLocationInGroup, i % channelGroupSize) == ChannelLocation::LEFT)
                      {
                         _leftChannels.push_back(&_channels[i]);
                      }
                    else
                      {
                         _rightChannels.push_back(&_channels[i]);
                      }
                 }
            }
        
        auto PatternReader::computeTickLength() const -> size_t
          {
             static constexpr size_t outputFrequency = toUnderlying(StandardFrequency::AMIGA);
             static constexpr size_t secondPerMinute = 60;
             static constexpr size_t linesPerBeat = 4;
             static constexpr size_t minuteLength = outputFrequency * secondPerMinute;
             
             const size_t beatLength = minuteLength / _bpm;
             
             const size_t lineLength = beatLength / linesPerBeat;
             return lineLength / _defaultSpeed;
          }
        
        auto PatternReader::isFinished() const -> bool
          {
             return _currentLine >= _numberOfLines || _patternJump;
          }
        
        auto PatternReader::getCurrentLine() const -> size_t
          {
             return _currentLine;
          }
        
        void PatternReader::setPattern(const mods::utils::RBuffer<Note>& patternBuffer, int initialLine)
          {
             _patternBuffer = patternBuffer;
             _currentTick = 0;
             _currentLine = initialLine;
             _patternJump = false;
          }
        
        void PatternReader::readNextTick()
          {
             if(_currentTick == 0)
               {
                  decodeLine();
               }
             else
               {
                  signalNewTick();
               }
             
             readAndMixTick(_leftOutput, _leftChannels);
             readAndMixTick(_rightOutput, _rightChannels);
             
             ++_currentTick;
             if(_currentTick == _speed)
               {
                  _currentTick = 0;
                  ++_currentLine;
                  
                  updateSpeed();
                  updatePatternJump();
                  updateLoop();
                  updateFilter();
               }
          }
        
        void PatternReader::readAndMixTick(OutputQueue* output, const std::vector<ChannelState*>& channels)
          {
             using mods::utils::AmigaRLESample;
             
             size_t toRead = computeTickLength();
             while(toRead > 0)
               {
                  auto rleSample = readAndMixSample(channels, toRead);
                  if(rleSample.getLength() > 0)
                    {
                       toRead -= rleSample.getLength();
                       output->push_back(rleSample);
                    }
                  else
                    {
                       output->push_back(RLESample(0, static_cast<AmigaRLESample::SampleLength>(toRead), false));
                       toRead = 0;
                    }
               }
          }
        
        auto PatternReader::readAndMixSample(const std::vector<ChannelState*>& channels, u32 maxLength) -> RLESample
          {
             using mods::utils::AmigaRLESample;
             
             u32 length = maxLength;
             for(auto* channel : channels)
               {
                  channel->prepareNextSample();
                  const u32 curSampleLength = channel->getCurrentSampleLength();
                  if(curSampleLength != 0 && curSampleLength < length)
                    {
                       length = curSampleLength;
                    }
               }
             double mixedSample = 0.0;
             for(auto* channel : channels)
               {
                  mixedSample += channel->readAndConsumeNextSample(length);
               }
             mixedSample /= static_cast<double>(channels.size());
             return {mixedSample, static_cast<AmigaRLESample::SampleLength>(length), false};
          }
        
        void PatternReader::decodeLine()
          {
             for(size_t i=0; i < _channels.size(); ++i)
               {
                  auto note = _patternBuffer.slice<Note>((_currentLine * _channels.size()) + i, 1);
                  _channels[i].updateChannelToNewLine(note);
               }
          }
        
        void PatternReader::signalNewTick()
          {
             for(auto& channel : _channels)
               {
                  channel.tick();
               }
          }
        
        void PatternReader::updateSpeed()
          {
             static constexpr int MAXIMUM_SPEED = 32;
             
             bool speedDefined = false;
             u32 speed = 0;
             
             for(auto& channel : _channels)
               {
                  if(channel.hasSpeedDefined())
                    {
                       speedDefined = true,
                       speed = channel.getSpeed();
                    }
               }
             
             if(speedDefined)
               {
                  if(speed > MAXIMUM_SPEED)
                    {
                       _bpm = speed;
                    }
                  else
                    {
                       _speed = speed;
                    }
               }
          }
        
        void PatternReader::updatePatternJump()
          {
             _patternJump = false;
             
             for(auto& channel : _channels)
               {
                  if(channel.hasPatternJump())
                    {
                       _patternJump = true;
                       _patternOfJumpTarget = channel.getPatternOfJumpTarget();
                       _lineOfJumpTarget = channel.getLineOfJumpTarget();
                    }
               }
          }
        
        void PatternReader::updateLoop()
          {
             for(auto& channel : _channels)
               {
                  if(channel.isStartOfLoop())
                    {
                       _startOfLoop = _currentLine - 1;
                    }
                  if(channel.isEndOfLoop())
                    {
                       if(_remainingLoop == 0)
                         {
                            _remainingLoop = channel.getLoopLength();
                         }
                       else
                         {
                            --_remainingLoop;
                         }
                       
                       if(_remainingLoop > 0)
                         {
                            _currentLine = _startOfLoop;
                         }
                    }
               }
          }
        
        auto PatternReader::hasPatternJump() const -> bool
          {
             return _patternJump;
          }
        
        auto PatternReader::getPatternOfJumpTarget() const -> int 
          {
             return _patternOfJumpTarget;
          }
        
        auto PatternReader::getLineOfJumpTarget() const -> int
          {
             return _lineOfJumpTarget;
          }
        
        void PatternReader::updateFilter()
          {
             bool hasNewFilterValue = false;
             bool newFilterValue = false;
             
             for(auto& channel : _channels)
               {
                  if(channel.hasNewFilterValue())
                    {
                       hasNewFilterValue = true;
                       newFilterValue = channel.getNewFilterValue();
                    }
               }
             
             if(hasNewFilterValue)
               {
                  for(auto& channel : _channels)
                    {
                       channel.setFilterValue(newFilterValue);
                    }
               }
          }
     } // namespace mod
} // namespace mods
