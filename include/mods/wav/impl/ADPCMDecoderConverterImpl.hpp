#ifndef MODS_WAV_IMPL_ADPCMDECODERCONVERTERIMPL_HPP
#define MODS_WAV_IMPL_ADPCMDECODERCONVERTERIMPL_HPP

namespace mods
{
   namespace wav
     {
        namespace impl
          {
#pragma pack(push,1)
             template<int NBCHANNELS> struct ADPCMPreamble;
             
             template<>
               struct ADPCMPreamble<1>
               {
                private:
                  u8 blockPredictor;
                  s16le initialDelta;
                  s16le sample1;
                  s16le sample2;
                  
                public:
                  auto getBlockPredictor(size_t /* channel */) const noexcept -> u8
                    {
                       return blockPredictor;
                    }
                  
                  auto getSampleTMinus1(size_t /* channel */) const noexcept -> s16
                    {
                       return static_cast<s16>(sample1);
                    }
                  
                  auto getSampleTMinus2(size_t /* channel */) const noexcept -> s16
                    {
                       return static_cast<s16>(sample2);
                    }
                  
                  auto getInitialDelta(size_t /* channel */) const noexcept -> s16
                    {
                       return static_cast<s16>(initialDelta);
                    }
               };
             
             template<>
               struct ADPCMPreamble<2>
               {
                private:
                  mods::utils::PackedArray<u8,2> blockPredictor;
                  mods::utils::PackedArray<s16le,2> initialDelta;
                  mods::utils::PackedArray<s16le,2> sample1;
                  mods::utils::PackedArray<s16le,2> sample2;
		  
		public:
		  auto getBlockPredictor(size_t channel) const noexcept -> u8
		    {
		       assert(channel < 2);
		       return blockPredictor[channel];
		    }
		  
		  auto getSampleTMinus1(size_t channel) const noexcept -> s16
		    {
		       assert(channel < 2);
		       return static_cast<s16>(sample1[channel]);
		    }
		  
		  auto getSampleTMinus2(size_t channel) const noexcept -> s16
		    {
		       assert(channel < 2);
		       return static_cast<s16>(sample2[channel]);
		    }
		  
		  auto getInitialDelta(size_t channel) const noexcept -> s16
                    {
                       assert(channel < 2);
                       return static_cast<s16>(initialDelta[channel]);
                    }
               };
             
             struct ADPCMExtension
               {
                private:
                  u16le samplesPerBlock;
                  u16le numCoef;
                  
                public:
                  auto getSamplesPerBlock() const noexcept -> u16
                    {
                       return static_cast<u16>(samplesPerBlock);
                    }
                  
                  auto getNumCoef() const noexcept -> u16
                    {
                       return static_cast<u16>(numCoef);
                    }
               };
#pragma pack(pop)
             
             struct ChannelPreamble
               {
                  s16 sampleTMinus1;
                  s16 sampleTMinus2;
                  size_t predictor;
                  s16 initialDelta;
               };
	     
	     class ADPCMChannelDecoder
	       {
		public:
		  ADPCMChannelDecoder() = default;
		  ADPCMChannelDecoder(const ADPCMChannelDecoder&) = delete;
		  ADPCMChannelDecoder(ADPCMChannelDecoder&&) = delete;
		  auto operator=(const ADPCMChannelDecoder&) -> ADPCMChannelDecoder& = delete;
		  auto operator=(ADPCMChannelDecoder&&) -> ADPCMChannelDecoder& = delete;
		  ~ADPCMChannelDecoder() = default;
		  
		  void initDecoder(const ChannelPreamble& channelPreamble, const mods::utils::RBuffer<s16>& coefs);
		  auto getSampleTMinus2() const -> s16;
		  auto getSampleTMinus1() const -> s16;
		  auto decodeSample(u8 sample) -> s16;
		  
		private:
		  s16 _sampleTMinus1 = 0;
		  s16 _sampleTMinus2 = 0;
		  s32 _coef1 = 0;
		  s32 _coef2 = 0;
		  s16 _delta = 0;
	       };
	  } // namespace impl
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_IMPL_ADPCMDECODERCONVERTERIMPL_HPP
