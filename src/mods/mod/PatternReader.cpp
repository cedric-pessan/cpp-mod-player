
#include "mods/mod/Note.hpp"
#include "mods/mod/PatternReader.hpp"
#include "mods/StandardFrequency.hpp"

#include <iostream>

namespace mods
{
   namespace mod
     {
        namespace
          {
             enum struct ChannelLocation
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
                                     const std::vector<mods::utils::RBuffer<u8>>& sampleBuffers,
                                     OutputQueue* leftOutput,
                                     OutputQueue* rightOutput)
          : _patternBuffer(patternBuffer),
          _leftOutput(leftOutput),
          _rightOutput(rightOutput)
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
             
             size_t minuteLength = outputFrequency * secondPerMinute;
             
             size_t beatLength = minuteLength / _bpm;
             
             size_t lineLength = beatLength / linesPerBeat;
             return lineLength / _speed;
          }
        
        auto PatternReader::isFinished() const -> bool
          {
             return _currentLine >= _numberOfLines;
          }
        
        auto PatternReader::getCurrentLine() const -> size_t
          {
             return _currentLine;
          }
        
        void PatternReader::setPattern(const mods::utils::RBuffer<Note>& patternBuffer)
          {
             _patternBuffer = patternBuffer;
             _currentTick = 0;
             _currentLine = 0;
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
               }
          }
        
        void PatternReader::readAndMixTick(OutputQueue* output, const std::vector<ChannelState*>& channels)
          {
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
                       output->push_back(RLESample(0, toRead, false));
                       toRead = 0;
                    }
               }
          }
        
        auto PatternReader::readAndMixSample(const std::vector<ChannelState*>& channels, u32 maxLength) const -> RLESample
          {
             u32 length = maxLength;
             for(auto* channel : channels)
               {
                  channel->prepareNextSample();
                  u32 l = channel->getCurrentSampleLength();
                  if(l != 0 && l < length)
                    {
                       length = l;
                    }
               }
             double mixedSample = 0.0;
             for(auto* channel : channels)
               {
                  mixedSample += channel->readAndConsumeNextSample(length);
               }
             mixedSample /= channels.size();
             return RLESample(mixedSample, length, false);
          }
        
        void PatternReader::decodeLine()
          {
             for(size_t i=0; i < _channels.size(); ++i)
               {
                  auto note = _patternBuffer.slice<Note>(_currentLine * _channels.size() + i, 1);
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
             bool speedDefined = false;
             int speed;
             
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
                  if(speed > 32)
                    _bpm = speed;
                  else
                    _speed = speed;
               }
          }
     } // namespace mod
} // namespace mods
