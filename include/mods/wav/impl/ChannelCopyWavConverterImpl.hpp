#ifndef MODS_WAV_IMPL_CHANNELCOPYWAVCONVERTERIMPL_HPP
#define MODS_WAV_IMPL_CHANNELCOPYWAVCONVERTERIMPL_HPP

namespace mods
{
   namespace wav
     {
        namespace impl
          {
             class InternalSourceConverter
               {
                public:
                  using sptr = std::shared_ptr<InternalSourceConverter>;
                  
                  InternalSourceConverter() = default;
                  InternalSourceConverter(const InternalSourceConverter&) = delete;
                  InternalSourceConverter(const InternalSourceConverter&&) = delete;
                  InternalSourceConverter& operator=(const InternalSourceConverter&) = delete;
                  InternalSourceConverter& operator=(const InternalSourceConverter&&) = delete;
                  ~InternalSourceConverter() = default;
                  
                  bool isFinished() const;
               };
             
             class ChannelCopyWavConverterSlave : public WavConverter
               {
                protected:
                  ChannelCopyWavConverterSlave(const InternalSourceConverter::sptr& src);
             
                public:
                  ChannelCopyWavConverterSlave() = delete;
                  ChannelCopyWavConverterSlave(const ChannelCopyWavConverterSlave&) = delete;
                  ChannelCopyWavConverterSlave(const ChannelCopyWavConverterSlave&&) = delete;
                  ChannelCopyWavConverterSlave& operator=(const ChannelCopyWavConverterSlave&) = delete;
                  ChannelCopyWavConverterSlave& operator=(const ChannelCopyWavConverterSlave&&) = delete;
                  ~ChannelCopyWavConverterSlave() override = default;
                  
                  bool isFinished() const override;
                  
                protected:
                  WavConverter::ptr buildSlave() const;
             
                private:
                  InternalSourceConverter::sptr _src;
               };
          } // namespace impl
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_IMPL_CHANNELCOPYWAVCONVERTERIMPL_HPP
