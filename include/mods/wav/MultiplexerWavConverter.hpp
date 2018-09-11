#ifndef MODS_WAV_MULTIPLEXERWAVCONVERTER_HPP
#define MODS_WAV_MULTIPLEXERWAVCONVERTER_HPP

#include "mods/wav/WavConverter.hpp"

namespace mods
{
   namespace wav
     {
        class MultiplexerWavConverter : public WavConverter
          {
           public:
             MultiplexerWavConverter(WavConverter::ptr&& left, WavConverter::ptr&& right);
             
             MultiplexerWavConverter() = delete;
             MultiplexerWavConverter(const MultiplexerWavConverter&) = delete;
             MultiplexerWavConverter(const MultiplexerWavConverter&&) = delete;
             MultiplexerWavConverter& operator=(const MultiplexerWavConverter&) = delete;
             MultiplexerWavConverter& operator=(const MultiplexerWavConverter&&) = delete;
             ~MultiplexerWavConverter() override = default;
             
             virtual bool isFinished() const;
             
           private:
             WavConverter::ptr _left;
             WavConverter::ptr _right;
          };
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_MULTIPLEXERWAVCONVERTER_HPP
