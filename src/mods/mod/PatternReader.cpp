
#include "mods/mod/PatternReader.hpp"

#include <iostream>

namespace mods
{
   namespace mod
     {
        PatternReader::PatternReader()
          : _tickBuffer(allocateTickBuffer(computeTickBufferLength())),
          _unreadTickBuffer(_tickBuffer.slice<s16>(0, 0))
            {
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
             std::cout << "TODO: PatternReader::computeTickBufferLength() const" << std::endl;
             return 0;
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
             std::cout << "TODO: PatternReader::readNextTick()" << std::endl;
          }
     } // namespace mod
} // namespace mods
