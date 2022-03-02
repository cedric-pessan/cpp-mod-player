
#include "mods/mod/Note.hpp"
#include "mods/mod/PatternReader.hpp"

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
                                     const std::vector<mods::utils::RBuffer<u8>>& sampleBuffers)
          : _tickBuffer(allocateTickBuffer(computeTickBufferLength())),
          _unreadTickBuffer(_tickBuffer.slice<s16>(0, 0)),
          _patternBuffer(patternBuffer)
            {
               using mods::utils::at;
               
               for(size_t i=0; i<nbChannels; ++i)
                 {
                    _channels.emplace_back(sampleBuffers);
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
        
        auto PatternReader::allocateTickBuffer(size_t len) -> mods::utils::RWBuffer<s16>
          {
             _tickVec.resize(len * sizeof(s16));
             u8* ptr = _tickVec.data();
             auto deleter = std::make_unique<mods::utils::RWBufferBackend::EmptyDeleter>();
             auto buffer = std::make_unique<mods::utils::RWBufferBackend>(ptr, len, std::move(deleter));
             return mods::utils::RWBuffer<s16>(std::move(buffer));
          }
        
        auto PatternReader::computeTickBufferLength() const -> size_t
          {
             static constexpr size_t outputFrequency = 44100;
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
        
        auto PatternReader::isTickFinished() const -> bool
          {
             return _unreadTickBuffer.empty();
          }
        
        auto PatternReader::getCurrentLine() const -> size_t
          {
             return _currentLine;
          }
        
        auto PatternReader::readTickBuffer(size_t nbElems) -> mods::utils::RBuffer<s16>
          {
             auto size = nbElems;
             if(size > _unreadTickBuffer.size())
               {
                  size = _unreadTickBuffer.size();
               }
             auto buf = _unreadTickBuffer.slice<s16>(0, size);
             _unreadTickBuffer = _unreadTickBuffer.slice<s16>(size, _unreadTickBuffer.size() - size);
             return buf;
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
             
             bool left = true;
             for(s16& outputSample : _tickBuffer)
               {
                  if(left)
                    {
                       outputSample = readAndMixSample(_leftChannels);
                    }
                  else
                    {
                       outputSample = readAndMixSample(_rightChannels);
                    }
                  left = !left;
               }
             
             _unreadTickBuffer = _tickBuffer.slice<s16>(0, _tickBuffer.size());
             ++_currentTick;
             if(_currentTick == _speed)
               {
                  _currentTick = 0;
                  ++_currentLine;
               }
          }
        
        auto PatternReader::readAndMixSample(const std::vector<ChannelState*>& channels) const -> s16
          {
             s32 sample = 0;
             for(auto* channel : channels)
               {
                  sample += channel->readNextSample();
               }
             sample /= channels.size();
             return static_cast<s16>(sample);
          }
        
        void PatternReader::decodeLine()
          {
             for(size_t i=0; i < _channels.size(); ++i)
               {
                  auto note = _patternBuffer.slice<Note>(_currentLine * _channels.size() + i, 1);
                  _channels[i].updateChannelToNewLine(note);
               }
          }
     } // namespace mod
} // namespace mods
