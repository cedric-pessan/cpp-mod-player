
#include "mods/mod/PatternListReader.hpp"

#include <iostream>
#include <sstream>

namespace mods
{
   namespace mod
     {
        PatternListReader::PatternListReader(size_t numberOfPatterns,
                                             const mods::utils::RBuffer<u8>& patternsOrderList,
                                             size_t nbChannels,
                                             const mods::utils::RBuffer<Note>& patterns,
                                             const mods::utils::RBuffer<Instrument>& instruments,
                                             const std::vector<mods::utils::RBuffer<u8>>& sampleBuffers)
          : _numberOfPatterns(numberOfPatterns),
          _patternsOrderList(patternsOrderList),
          _nbChannels(nbChannels),
          _patterns(patterns),
          _patternReader(_nbChannels, getPatternBuffer(_currentPatternIndex), instruments, sampleBuffers,
                         &_unconsumedBuffers.at(toUnderlying(ChannelId::LEFT)),
                         &_unconsumedBuffers.at(toUnderlying(ChannelId::RIGHT)))
            {
            }
        
        auto PatternListReader::getProgressInfo() const -> std::string
          {
             std::stringstream ss;
             ss << "Pattern " << _currentPatternIndex << " / " << _numberOfPatterns << ", line " << _patternReader.getCurrentLine() << " / " << PatternReader::getNumberOfLines() << "     ";
             return ss.str();
          }
        
        auto PatternListReader::isChannelFinished(ChannelId channel) const -> bool
          {
             auto idxBuffer = toUnderlying(channel);
             if(!_unconsumedBuffers.at(idxBuffer).empty())
               {
                  return false;
               }
             return _currentPatternIndex >= _numberOfPatterns;
          }
        
        void PatternListReader::readChannel(mods::utils::RWBuffer<mods::utils::AmigaRLESample>* buf, ChannelId channel)
          {
             const size_t toRead = buf->size();
             size_t read = 0;
             auto idxBuffer = toUnderlying(channel);
             
             while(read < toRead && !isChannelFinished(channel))
               {
                  if(!_unconsumedBuffers.at(idxBuffer).empty())
                    {
                       auto& value = _unconsumedBuffers.at(idxBuffer).front();
                       (*buf)[read++] = value;
                       _unconsumedBuffers.at(idxBuffer).pop_front();
                    }
                  else if(!_patternReader.isFinished())
                    {
                       _patternReader.readNextTick();
                    }
                  else
                    {
                       ++_currentPatternIndex;
                       if(_currentPatternIndex < _numberOfPatterns)
                         {
                            _patternReader.setPattern(getPatternBuffer(_currentPatternIndex));
                         }
                    }
               }
          }
        
        auto PatternListReader::getPatternBuffer(size_t patternIndex) -> mods::utils::RBuffer<Note>
          {
             auto p = _patternsOrderList[patternIndex];
             auto patternBufferLength = _nbChannels * PatternReader::getNumberOfLines();
             return _patterns.slice<Note>(p * patternBufferLength, patternBufferLength);
          }
     } // namespace mod
} // namespace mods
