
#include "mods/mod/PatternReader.hpp"

#include <iostream>

namespace mods
{
   namespace mod
     {
        PatternReader::PatternReader(size_t nbChannels)
          : _tickBuffer(allocateTickBuffer(computeTickBufferLength())),
          _unreadTickBuffer(_tickBuffer.slice<s16>(0, 0))
            {
               for(size_t i=0; i<nbChannels; ++i)
                 {
                    _channels.emplace_back();
                 }
            }
        
        auto PatternReader::allocateTickBuffer(size_t len) -> mods::utils::RWBuffer<s16>
          {
             _tickVec.resize(len * sizeof(s16));
             u8* ptr = _tickVec.data();
             auto deleter = std::make_unique<mods::utils::RWBufferBackend::EmptyDeleter>();
             auto buffer = std::make_shared<mods::utils::RWBufferBackend>(ptr, len, std::move(deleter));
             return mods::utils::RWBuffer<s16>(buffer);
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
             return _currentLine >= _numberOfLines && isLineFinished();
          }
        
        auto PatternReader::isLineFinished() const -> bool
          {
             return _currentTick >= _speed && isTickFinished();
          }
        
        auto PatternReader::isTickFinished() const -> bool
          {
             return _unreadTickBuffer.empty();
          }
        
        auto PatternReader::readTickBuffer(size_t nbElems) const -> mods::utils::RBuffer<s16>
          {
             std::cout << "TODO: PatternReader::readTickBuffer(size_t) const" << std::endl;
             return mods::utils::RBuffer<s16>(nullptr);
          }
        
        void PatternReader::setPattern()
          {
             std::cout << "TODO: PatternReader::setPattern()" << std::endl;
          }
        
        void PatternReader::readNextTick()
          {
             bool left = true;
             for(s16& outputSample : _tickBuffer)
               {
                  if(left)
                    {
                       outputSample = readAndMixSample();
                    }
                  else
                    {
                       outputSample = readAndMixSample();
                    }
                  left = !left;
               }
             
             _unreadTickBuffer = _tickBuffer.slice<s16>(0, _tickBuffer.size());
          }
        
        auto PatternReader::readAndMixSample() const -> s16
          {
             std::cout << "TODO: PatternReader::readAndMixSample()" << std::endl;
             return 0;
          }
     } // namespace mod
} // namespace mods
