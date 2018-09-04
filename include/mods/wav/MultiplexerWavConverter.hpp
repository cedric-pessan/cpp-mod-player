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
             MultiplexerWavConverter() = default;
             MultiplexerWavConverter(const MultiplexerWavConverter&) = delete;
             MultiplexerWavConverter(const MultiplexerWavConverter&&) = delete;
             MultiplexerWavConverter& operator=(const MultiplexerWavConverter&) = delete;
             MultiplexerWavConverter& operator=(const MultiplexerWavConverter&&) = delete;
             ~MultiplexerWavConverter() override = default;
             
             virtual bool isFinished() const;
          };
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_MULTIPLEXERWAVCONVERTER_HPP
