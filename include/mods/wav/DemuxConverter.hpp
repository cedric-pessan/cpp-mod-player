#ifndef MODS_WAV_DEMUXCONVERTER_HPP
#define MODS_WAV_DEMUXCONVERTER_HPP

#include "mods/wav/impl/DemuxConverterImpl.hpp"

namespace mods
{
   namespace wav
     {
        class DemuxConverter : public impl::DemuxConverterSlave
          {
           public:
             DemuxConverter(Converter::ptr src, u32 nbChannels, u32 bitsPerContainer);
             
             DemuxConverter() = delete;
             DemuxConverter(const DemuxConverter&) = delete;
             DemuxConverter(DemuxConverter&&) = delete;
             auto operator=(const DemuxConverter&) = delete;
             auto operator=(DemuxConverter&&) = delete;
             ~DemuxConverter() override = default;
             
             auto getFirstChannels() -> std::vector<ptr>*;
             
           private:
             std::vector<ptr> _firstChannels;
          };
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_DEMUXCONVERTER_HPP
