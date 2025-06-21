
#include "mods/mod/ChannelId.hpp"
#include "mods/mod/Instrument.hpp"
#include "mods/mod/Note.hpp"
#include "mods/mod/PatternListReader.hpp"
#include "mods/mod/PatternReader.hpp"
#include "mods/utils/AmigaRLESample.hpp"
#include "mods/utils/RBuffer.hpp"
#include "mods/utils/RWBuffer.hpp"
#include "mods/utils/types.hpp"

#include <cstddef>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace mods
{
   namespace mod
     {
        PatternListReader::PatternListReader(size_t numberOfPatterns,
                                             const mods::utils::RBuffer<u8>& patternsOrderList,
                                             size_t nbChannels,
                                             const mods::utils::RBuffer<Note>& patterns,
                                             const mods::utils::RBuffer<Instrument>& instruments,
                                             const std::vector<mods::utils::RBuffer<s8>>& sampleBuffers)
          : _numberOfPatterns(numberOfPatterns),
          _patternsOrderList(patternsOrderList),
          _nbChannels(nbChannels),
          _patterns(patterns),
          _patternReader(_nbChannels, getPatternBuffer(_currentPatternIndex), instruments, sampleBuffers, &_unconsumedBuffers)
            {
            }
        
        auto PatternListReader::getProgressInfo() const -> std::string
          {
             std::stringstream infoStream;
             infoStream << "Pattern " << _currentPatternIndex << " / " << _numberOfPatterns << ", line " << _patternReader.getCurrentLine() << " / " << PatternReader::getNumberOfLines() << "     ";
             return infoStream.str();
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
                       const auto& value = _unconsumedBuffers.at(idxBuffer).front();
                       (*buf)[read++] = value;
                       _unconsumedBuffers.at(idxBuffer).pop_front();
                    }
                  else if(!_patternReader.isFinished())
                    {
                       _patternReader.readNextTick();
                    }
                  else
                    {
                       int initialLine = 0;
                       if(_patternReader.hasPatternJump())
                         {
                            const int patternOfJumpTarget = _patternReader.getPatternOfJumpTarget();
                            if(patternOfJumpTarget >= 0)
                              {
                                 _currentPatternIndex = patternOfJumpTarget;
                              }
                            else
                              {
                                 ++_currentPatternIndex;
                              }
                            initialLine = _patternReader.getLineOfJumpTarget();
                         }
                       else
                         {
                            ++_currentPatternIndex;
                         }
                       if(_currentPatternIndex < _numberOfPatterns)
                         {
                            _patternReader.setPattern(getPatternBuffer(_currentPatternIndex), initialLine);
                         }
                    }
               }
          }
        
        auto PatternListReader::getPatternBuffer(size_t patternIndex) -> mods::utils::RBuffer<Note>
          {
             auto pattern = _patternsOrderList[patternIndex];
             auto patternBufferLength = _nbChannels * PatternReader::getNumberOfLines();
             return _patterns.slice<Note>(pattern * patternBufferLength, patternBufferLength);
          }
     } // namespace mod
} // namespace mods
