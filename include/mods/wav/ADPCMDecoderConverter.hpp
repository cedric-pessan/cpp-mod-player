#ifndef MODS_WAV_ADPCMDECODERCONVERTER_HPP
#define MODS_WAV_ADPCMDECODERCONVERTER_HPP

#include "mods/wav/WavConverter.hpp"
#include "mods/wav/impl/ADPCMDecoderConverterImpl.hpp"

namespace mods
{
   namespace wav
     {
	class ADPCMDecoderConverter : public WavConverter
	  {
	   public:
	     ADPCMDecoderConverter(WavConverter::ptr src, int bitsPerContainer);
	     
	     ADPCMDecoderConverter() = delete;
	     ADPCMDecoderConverter(const ADPCMDecoderConverter&) = delete;
             ADPCMDecoderConverter(ADPCMDecoderConverter&&) = delete;
	     auto operator=(const ADPCMDecoderConverter&) -> ADPCMDecoderConverter& = delete;
	     auto operator=(ADPCMDecoderConverter&&) -> ADPCMDecoderConverter& = delete;
	     ~ADPCMDecoderConverter() override = default;
	     
	     auto isFinished() const -> bool override;
	     void read(mods::utils::RWBuffer<u8>* buf, size_t len) override;
             
             static constexpr auto getOutputBitsPerSample() -> int
               {
                  constexpr int outputBitsPerSample = 16;
                  return outputBitsPerSample;
               }
	     
	   private:
             static constexpr auto getDecodedBufferSize(int bitsPerContainer) -> int
               {
                  return bitsPerContainer - sizeof(impl::ADPCMPreamble) + 2;
               }
             
             template<typename ARRAY>
               static auto initializeRWBuffer(ARRAY& backArray) -> mods::utils::RWBuffer<typename ARRAY::value_type>;
             
             void decodeBlock();
	     /*auto decodeSample(int sample) -> s16;*/
	     
	     WavConverter::ptr _src;
             
             /*static constexpr u32 _defaultStepSize = 7;
	     
	     int _sample = 0;
	     u32 _stepSize = _defaultStepSize;
	     int _newSample = 0;
	     int _index = 0;*/
             
             std::vector<s16> _decodedBuffer;
             std::vector<s16>::const_iterator _itDecodedBuffer;
             
             std::vector<u8> _encodedVec;
             mods::utils::RWBuffer<u8> _encodedBuffer;
	  };
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_ADPCMDECODERCONVERTER_HPP
