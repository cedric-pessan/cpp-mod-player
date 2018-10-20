#ifndef MODS_WAV_MULTIPLEXERWAVCONVERTER_HPP
#define MODS_WAV_MULTIPLEXERWAVCONVERTER_HPP

#include "mods/wav/WavConverter.hpp"

#include <vector>

namespace mods
{
   namespace wav
     {
        class MultiplexerWavConverter : public WavConverter
          {
           public:
             MultiplexerWavConverter(WavConverter::ptr left, WavConverter::ptr right);
             
             MultiplexerWavConverter() = delete;
             MultiplexerWavConverter(const MultiplexerWavConverter&) = delete;
             MultiplexerWavConverter(const MultiplexerWavConverter&&) = delete;
             MultiplexerWavConverter& operator=(const MultiplexerWavConverter&) = delete;
             MultiplexerWavConverter& operator=(const MultiplexerWavConverter&&) = delete;
             ~MultiplexerWavConverter() override = default;
             
             bool isFinished() const override;
             void read(mods::utils::RWBuffer<u8>& buf, int len) override;
             
           private:
             mods::utils::RWBuffer<u8> allocateTempBuffer(int len);
             
             WavConverter::ptr _left;
             WavConverter::ptr _right;
             
             std::vector<u8> _tempVec;
             mods::utils::RWBuffer<u8> _temp;
          };
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_MULTIPLEXERWAVCONVERTER_HPP
