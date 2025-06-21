
#include "mods/utils/FileUtils.hpp"
#include "mods/utils/RBuffer.hpp"
#include "mods/utils/RWBuffer.hpp"
#include "mods/utils/optional.hpp"
#include "mods/utils/types.hpp"
#include "mods/wav/ADPCMDecoderConverter.hpp"
#include "mods/wav/ALawConverter.hpp"
#include "mods/wav/Format.hpp"
#include "mods/wav/GSMDecoderConverter.hpp"
#include "mods/wav/MuLawConverter.hpp"
#include "mods/wav/WavConverterFactory.hpp"
#include "mods/wav/WavReader.hpp"
#include "mods/wav/WavTypes.hpp"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>

namespace mods
{
   namespace wav
     {
        namespace
          {
             auto getRIFF() -> const std::string&
               {
                  static const std::string RIFF = "RIFF";
                  return RIFF;
               }
             auto getWAVE() -> const std::string&
               {
                  static const std::string WAVE = "WAVE";
                  return WAVE;
               }
             auto getFMT() -> const std::string&
               {
                  static const std::string FMT = "fmt ";
                  return FMT;
               }
             auto getFACT() -> const std::string&
               {
                  static const std::string FACT = "fact";
                  return FACT;
               }
             auto getDATA() -> const std::string&
               {
                  static const std::string DATA = "data";
                  return DATA;
               }
             auto getDISP() -> const std::string&
               {
                  static const std::string DISP = "DISP";
                  return DISP;
               }
             auto getLIST() -> const std::string&
               {
                  static const std::string LIST = "LIST";
                  return LIST;
               }
             auto getINFO() -> const std::string&
               {
                  static const std::string INFO = "INFO";
                  return INFO;
               }
             auto getICOP() -> const std::string&
               {
                  static const std::string ICOP = "ICOP";
                  return ICOP;
               }
             auto getISBJ() -> const std::string&
               {
                  static const std::string ISBJ = "ISBJ";
                  return ISBJ;
               }
             auto getICRD() -> const std::string&
               {
                  static const std::string ICRD = "ICRD";
                  return ICRD;
               }
             auto getISFT() -> const std::string&
               {
                  static const std::string ISFT = "ISFT";
                  return ISFT;
               }
             auto getAfsp() -> const std::string&
               {
                  static const std::string afsp = "afsp";
                  return afsp;
               }
             auto getICMT() -> const std::string&
               {
                  static const std::string ICMT = "ICMT";
                  return ICMT;
               }
             auto getCUE() -> const std::string&
               {
                  static const std::string CUE = "cue ";
                  return CUE;
               }
             auto getPEAK() -> const std::string&
               {
                  static const std::string PEAK = "PEAK";
                  return PEAK;
               }
             auto getADTL() -> const std::string&
               {
                  static const std::string ADTL = "adtl";
                  return ADTL;
               }
             auto getIPRD() -> const std::string&
               {
                  static const std::string IPRD = "IPRD";
                  return IPRD;
               }
          } // namespace
        
        WavReader::WavReader(const std::string& filename)
          : _fileBuffer(mods::utils::FileUtils::mapFile(filename))
            {
               std::stringstream description;
               
               auto headerBuffer = _fileBuffer.slice<ChunkHeader>(0, 1);
               
               checkInit(headerBuffer->getChunkID() == getRIFF(), "Not a RIFF file");
               bool incompleteRiff = false;
               
               auto riffChunkSize = headerBuffer->getChunkSize();
               if((riffChunkSize & 1U) != 0) // padding
                 {
                    ++riffChunkSize;
                 }
               if(_fileBuffer.size() < riffChunkSize + sizeof(ChunkHeader))
                 {
                    incompleteRiff = true;
                 }
               
               auto riffHeader = _fileBuffer.slice<RiffHeader>(0, 1);
               
               checkInit(riffHeader->getFormat() == getWAVE(), "Not a WAVE file");
               
               auto riffLength = incompleteRiff ? 
                 _fileBuffer.size() - sizeof(RiffHeader) :
                 riffChunkSize - sizeof(RiffHeader) + sizeof(ChunkHeader);
               auto riffBuffer = _fileBuffer.slice<u8>(sizeof(RiffHeader), riffLength);
               
               using mods::utils::RBuffer;
               optional<Format> optFmt;
               optional<RBuffer<FactHeader>> optFactHeader;
               optional<RBuffer<u8>> optData;
               double peak = 1.0;
               
               size_t offset = 0;
               while(offset < riffBuffer.size())
                 {
                    if((riffBuffer.size() - offset) < sizeof(ChunkHeader))
                      {
                         std::cout << "Warning: Incomplete last chunk" << '\n';
                         break;
                      }
                    auto chunkHeader = riffBuffer.slice<ChunkHeader>(offset, 1);
                    
                    if(chunkHeader->getChunkID() == getFMT())
                      {
                         checkInit(!optFmt.has_value(), "Multiple fmt chunks defined");
                         optFmt = readFMT(chunkHeader, riffBuffer, offset);
                      }
                    else if(chunkHeader->getChunkID() == getFACT())
                      {
                         checkInit(!optFactHeader.has_value(), "Multiple fact chunks defined");
                         optFactHeader = readFact(chunkHeader, riffBuffer, offset);
                      }
                    else if(chunkHeader->getChunkID() == getDATA())
                      {
                         checkInit(!optData.has_value(), "Multiple data chunks defined");
                         optData = readData(chunkHeader, riffBuffer, offset);
                      }
                    else if(chunkHeader->getChunkID() == getDISP())
                      {
                         readDisp(chunkHeader, riffBuffer, offset, description);
                      }
                    else if(chunkHeader->getChunkID() == getLIST())
                      {
                         readListChunk(chunkHeader, riffBuffer, offset, description);
                      }
                    else if(chunkHeader->getChunkID() == getAfsp())
                      {
                         readAfsp(chunkHeader, riffBuffer, offset, description);
                      }
                    else if(chunkHeader->getChunkID() == getCUE())
                      {
                         readCue();
                      }
                    else if(chunkHeader->getChunkID() == getPEAK())
                      {
                         const int nbChannels = optFmt.has_value() ? (*optFmt).getNumChannels() : 0;
                         peak = readPeak(chunkHeader, riffBuffer, offset, nbChannels);
                      }
                    else
                      {
                         std::cout << "Warning: Unknown RIFF chunk: " << chunkHeader->getChunkID() << '\n';
                      }
                    
                    auto chunkSize = chunkHeader->getChunkSize();
                    if((chunkSize & 1U) != 0) // padding
                      {
                         ++chunkSize;
                      }
                    offset += chunkSize + sizeof(ChunkHeader);
                 }
               
               checkInit(optFmt.has_value() && optData.has_value(), "Wav file should have at least a fmt and data chunk");
               
               const auto& fmt = *optFmt;
               const auto& data = *optData;
               _converter = WavConverterFactory::buildConverter(data, fmt, &_statCollector, peak);
               _length = data.size();
               
               buildInfo(fmt, description.str());
            }
        
        auto WavReader::readFMT(const mods::utils::RBuffer<ChunkHeader>& chunkHeader,
                                const mods::utils::RBuffer<u8>& riffBuffer,
                                size_t offset) -> Format
          {
             checkInit(chunkHeader->getChunkSize() <= riffBuffer.size() - offset - sizeof(ChunkHeader) &&
                       chunkHeader->getChunkSize() >= sizeof(FmtHeader) - sizeof(ChunkHeader) , "Incomplete FMT chunk");
             
             auto fmtHeader = riffBuffer.slice<FmtHeader>(offset, 1);
             
             optional<mods::utils::RBuffer<ExtendedFmtHeader>> extendedFmtHeader;
             
             if(fmtHeader->getChunkHeader().getChunkSize() >= sizeof(ExtendedFmtHeader) - sizeof(ChunkHeader))
               {
                  mods::utils::RBuffer<ExtendedFmtHeader> buf = riffBuffer.slice<ExtendedFmtHeader>(offset, 1);
                  extendedFmtHeader = buf;
               }
             
             optional<mods::utils::RBuffer<ExtensibleHeader>> extensibleHeader;
             
             if(extendedFmtHeader.has_value() &&
                fmtHeader->getChunkHeader().getChunkSize() >= sizeof(ExtensibleHeader) - sizeof(ChunkHeader) &&
                (*extendedFmtHeader)->getExtensionSize() == (sizeof(ExtensibleHeader) - sizeof(ExtendedFmtHeader)) &&
                fmtHeader->getAudioFormat() == WavAudioFormat::EXTENSIBLE)
               {
                  mods::utils::RBuffer<ExtensibleHeader> buf = riffBuffer.slice<ExtensibleHeader>(offset, 1);
                  extensibleHeader = buf;
               }
             
             bool useChannelMask = false;
             auto format = fmtHeader->getAudioFormat();
             if(format == WavAudioFormat::EXTENSIBLE)
               {
                  if(extensibleHeader.has_value())
                    {
                       const auto& ext = *extensibleHeader;
                       format = ext->getAudioFormat();
                       
                       if((fmtHeader->getNumChannels() != 1 || ext->getChannelMask() != 1) && (fmtHeader->getNumChannels() != 2 || ext->getChannelMask() != 3) && ext->getChannelMask() != 0)
                         {
                            useChannelMask = true;
                         }
                    }
               }
             
             optional<mods::utils::RBuffer<u8>> metadataExtension;
             
             if(extendedFmtHeader.has_value() &&
                fmtHeader->getChunkHeader().getChunkSize() >= sizeof(ExtendedFmtHeader) + (*extendedFmtHeader)->getExtensionSize() - sizeof(ChunkHeader))
               {
                  mods::utils::RBuffer<u8> buf = riffBuffer.slice<u8>(offset + sizeof(ExtendedFmtHeader), (*extendedFmtHeader)->getExtensionSize());
                  metadataExtension = buf;
               }
             
             Format decodedFormat(std::move(fmtHeader), std::move(extensibleHeader), useChannelMask, std::move(metadataExtension));
             
             switch(format) 
               {
                case WavAudioFormat::PCM:
                  break;
                  
                case WavAudioFormat::A_LAW:
                  checkInit(ALawConverter::isValidAsBitsPerSample(decodedFormat.getBitsPerSample()), ALawConverter::getBitsPerSampleRequirementsString());
                  break;
                  
                case WavAudioFormat::MU_LAW:
                  checkInit(MuLawConverter::isValidAsBitsPerSample(decodedFormat.getBitsPerSample()), MuLawConverter::getBitsPerSampleRequirementsString());
                  break;
                  
                case WavAudioFormat::IEEE_FLOAT:
                  checkInit(decodedFormat.getBitsPerSample() == sizeof(u32) * BITS_IN_BYTE || decodedFormat.getBitsPerSample() == sizeof(u64) * BITS_IN_BYTE, "IEEE float codec needs 32 or 64 bits per sample");
                  break;
                  
                case WavAudioFormat::TRUSPEECH:
                  break;
                  
                case WavAudioFormat::GSM:
                  checkInit(GSMDecoderConverter::isValidAsBitsPerSample(decodedFormat.getBitsPerSample()), GSMDecoderConverter::getBitsPerSampleRequirementsString());
                  checkInit(extendedFmtHeader.has_value(), "GSM codec without extended fmt");
                  break;
                  
                case WavAudioFormat::OKI_ADPCM:
                  break;
                  
		case WavAudioFormat::DVI_ADPCM:
		  checkInit(decodedFormat.getBitsPerSample() == 4, "Samples should be 4 bits for DVI/ADPCM");
		  checkInit((decodedFormat.getBitsPerContainer() % 4) == 0, "Container length for DVI/ADPCM should be a multiple of 4");
		  break;
                  
                case WavAudioFormat::ADPCM:
                  if(decodedFormat.getNumChannels() == 2)
                    {
                       checkInit(ADPCMDecoderConverter<2>::isExtensionSizeValid(decodedFormat.getMetaData().size()), "Invalid extension size for ADPCM decoder");
                    }
                  else
                    {
                       checkInit(ADPCMDecoderConverter<1>::isExtensionSizeValid(decodedFormat.getMetaData().size()), "Invalid extension size for ADPCM decoder");
                    }
                  break;
                  
                default:
                    {
                       std::stringstream sStream;
                       sStream << "Codec not supported: " << std::hex << decodedFormat.getAudioFormatAsNumber() << std::dec;
                       checkInit(false, sStream.str());
                    }
               }
             
             return decodedFormat;
          }
        
        auto WavReader::readFact(const mods::utils::RBuffer<ChunkHeader>& chunkHeader,
                                 const mods::utils::RBuffer<u8>& riffBuffer,
                                 size_t offset) -> mods::utils::RBuffer<FactHeader>
          {
             checkInit(chunkHeader->getChunkSize() <= riffBuffer.size() - offset - sizeof(ChunkHeader) &&
                       chunkHeader->getChunkSize() == sizeof(FactHeader) - sizeof(ChunkHeader) , "Incomplete Fact chunk");
             
             auto factHeader = riffBuffer.slice<FactHeader>(offset, 1);
             
             return factHeader;
          }
        
        auto WavReader::readData(const mods::utils::RBuffer<ChunkHeader>& chunkHeader,
                                 const mods::utils::RBuffer<u8>& riffBuffer,
                                 size_t offset) -> mods::utils::RBuffer<u8>
          {
             checkInit(chunkHeader->getChunkSize() <= riffBuffer.size() - offset - sizeof(ChunkHeader),
                       "Incomplete Data chunk");
             
             auto data = riffBuffer.slice<u8>(offset + sizeof(ChunkHeader), chunkHeader->getChunkSize());
             
             return data;
          }
        
        void WavReader::readDisp(const mods::utils::RBuffer<ChunkHeader>& chunkHeader,
                                 const mods::utils::RBuffer<u8>& riffBuffer,
                                 size_t offset,
                                 std::stringstream& description)
          {
             checkInit(chunkHeader->getChunkSize() <= riffBuffer.size() - offset - sizeof(ChunkHeader) &&
                       chunkHeader->getChunkSize() >= sizeof(DispHeader), "Incomplete Disp chunk");
             
             auto disp = riffBuffer.slice<DispHeader>(offset, 1);
             
             if(disp->getType() == DispType::TEXT)
               {
                  auto payload = riffBuffer.slice<u8>(offset + sizeof(DispHeader), chunkHeader->getChunkSize() - (sizeof(DispHeader) - sizeof(ChunkHeader)));
                  
                  if(!description.str().empty())
                    {
                       description << '\n';
                    }
                  
                  for(auto byte : payload)
                    {
                       if(byte == '\0')
                         {
                            break;
                         }
                       description << byte;
                    }
               }
          }
        
        void WavReader::readListChunk(const mods::utils::RBuffer<ChunkHeader>& chunkHeader,
                                      const mods::utils::RBuffer<u8>& riffBuffer,
                                      size_t offset,
                                      std::stringstream& description)
          {
             checkInit(chunkHeader->getChunkSize() <= riffBuffer.size() - offset - sizeof(ChunkHeader) &&
                       chunkHeader->getChunkSize() >= sizeof(ListHeader) - sizeof(ChunkHeader) , "Incomplete LIST chunk");
             
             auto listHeader = riffBuffer.slice<ListHeader>(offset, 1);
             
             if(listHeader->getListTypeID() == getINFO())
               {
                  parseInfoList(listHeader, riffBuffer, offset, description);
               }
             else if(listHeader->getListTypeID() == getADTL())
               {
                  parseAdtl();
               }
             else
               {
                  std::stringstream sStream;
                  sStream << "Unknown LIST chunk: " << listHeader->getListTypeID();
                  checkInit(false, sStream.str());
               }
          }
        
        void WavReader::readAfsp(const mods::utils::RBuffer<ChunkHeader>& chunkHeader,
                                 const mods::utils::RBuffer<u8>& riffBuffer,
                                 size_t offset,
                                 std::stringstream& description)
          {
             checkInit(chunkHeader->getChunkSize() <= riffBuffer.size() - offset - sizeof(ChunkHeader) &&
                       chunkHeader->getChunkSize() >= sizeof(AfspHeader), "Incomplete Afsp chunk");
             
             auto stringBuffer = riffBuffer.slice<char>(offset + sizeof(AfspHeader), chunkHeader->getChunkSize() - (sizeof(AfspHeader) - sizeof(ChunkHeader)));
             std::string infos = std::string(stringBuffer.begin(), stringBuffer.end());
             for(size_t i = 0; i < infos.length(); ++i)
               {
                  if(infos[i] == '\0' && i != infos.length()-1)
                    {
                       infos[i] = '\n';
                    }
               }
             if(!description.str().empty())
               {
                  description << '\n';
               }
             description << "Afsp infos:" << infos;
          }
        
        void WavReader::readCue() const
          {
             // no need to read cue until we handle play lists
          }
        
        void WavReader::parseAdtl()
          {
             // no need to read adtl until we handle play lists and cue
          }
        
        auto WavReader::readPeak(const mods::utils::RBuffer<ChunkHeader>& chunkHeader,
                                 const mods::utils::RBuffer<u8>& riffBuffer,
                                 size_t offset,
                                 int nbChannels) -> double
          {
             if(nbChannels == 0)
               {
                  std::cout << "Peak chunk defined before number of channels is known" << '\n';
               }
             
             checkInit(chunkHeader->getChunkSize() <= riffBuffer.size() - offset - sizeof(ChunkHeader) &&
                       chunkHeader->getChunkSize() == sizeof(PeakHeader) - sizeof(ChunkHeader) + nbChannels * sizeof(PPeakHeader), "Incomplete Peak chunk");
             
             auto ppeak = riffBuffer.slice<PPeakHeader>(offset + sizeof(PeakHeader), nbChannels);
             
             float maxPeak = 0.0;
             
             for(int i=0; i<nbChannels; ++i)
               {
                  maxPeak = std::max(ppeak[i].value, maxPeak);
               }
             
             return maxPeak;
          }
        
        void WavReader::parseInfoList(const mods::utils::RBuffer<ListHeader>& listHeader,
                                      const mods::utils::RBuffer<u8>& riffBuffer,
                                      size_t offset,
                                      std::stringstream& description)
          {
             auto listBuffer = riffBuffer.slice<u8>(offset + sizeof(ListHeader), listHeader->getChunkHeader().getChunkSize() - (sizeof(ListHeader) - sizeof(ChunkHeader)));
             size_t listOffset = 0;
             while(listOffset < listBuffer.size())
               {
                  auto chunkHeader = listBuffer.slice<ChunkHeader>(listOffset, 1);
                  auto stringBuffer = listBuffer.slice<char>(listOffset + sizeof(ChunkHeader), chunkHeader->getChunkSize());
                  const std::string info(stringBuffer.begin(), stringBuffer.end());
                  if(!description.str().empty())
                    {
                       description << '\n';
                    }
                  
                  if(chunkHeader->getChunkID() == getICOP())
                    {
                       description << "copyright:";
                    }
                  else if(chunkHeader->getChunkID() == getISBJ())
                    {
                       description << "subject:";
                    }
                  else if(chunkHeader->getChunkID() == getICRD())
                    {
                       description << "creation date:";
                    }
                  else if(chunkHeader->getChunkID() == getISFT())
                    {
                       description << "software:";
                    }
                  else if(chunkHeader->getChunkID() == getICMT())
                    {
                       description << "comment:";
                    }
                  else if(chunkHeader->getChunkID() == getIPRD())
                    {
                       description << "title:";
                    }
                  else
                    {
                       std::stringstream sStream;
                       sStream << "Unknown Info chunk: " << chunkHeader->getChunkID();
                       checkInit(false, sStream.str());
                    }
                  description << info;
                  
                  auto chunkSize = chunkHeader->getChunkSize();
                  if((chunkSize & 1U) != 0) // padding
                    {
                       ++chunkSize;
                    }
                  listOffset += chunkSize + sizeof(ChunkHeader);
               }
          }
        
        void WavReader::buildInfo(const Format& format, const std::string& description)
          {
             std::stringstream sStream;
             if(!description.empty())
               {
                  sStream << "description: " << description << '\n';
               }
             sStream << "bits per container:" << format.getBitsPerContainer() << '\n';
             sStream << "bits per sample: " << format.getBitsPerSample() << '\n';
             sStream << "number of channels: " << format.getNumChannels() << '\n';
             sStream << "frequency: " << format.getSampleRate() << '\n';
             sStream << "codec: " << toString(format.getAudioFormat());
             _info = sStream.str();
          }
        
        auto WavReader::isFinished() const -> bool
          {
             return _converter->isFinished();
          }
        
        void WavReader::read(mods::utils::RWBuffer<s16>* buf)
          {
             _converter->read(buf);
          }
        
        auto WavReader::getInfo() const -> std::string
          {
             return _info;
          }
        
        auto WavReader::getProgressInfo() const -> std::string
          {
             static constexpr int fullProgressValue = 100;
             
             const size_t read = _statCollector.getBytesRead();
             const size_t percentage = read * fullProgressValue / _length;
             std::stringstream sStream;
             sStream << "Reading..." << percentage << "%";
             return sStream.str();
          }
     } // namespace wav
} // namespace mods
