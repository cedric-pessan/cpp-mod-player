
#include "mods/wav/Format.hpp"
#include "mods/wav/WavReader.hpp"
#include "mods/utils/FileUtils.hpp"
#include "mods/utils/optional.hpp"

#include <iostream>
#include <sstream>

namespace mods
{
   namespace wav
     {
        namespace
          {
             const std::string& getRIFF()
               {
                  static const std::string RIFF = "RIFF";
                  return RIFF;
               }
             const std::string& getWAVE()
               {
                  static const std::string WAVE = "WAVE";
                  return WAVE;
               }
             const std::string& getFMT()
               {
                  static const std::string FMT = "fmt ";
                  return FMT;
               }
             const std::string& getFACT()
               {
                  static const std::string FACT = "fact";
                  return FACT;
               }
             const std::string& getDATA()
               {
                  static const std::string DATA = "data";
                  return DATA;
               }
             const std::string& getDISP()
               {
                  static const std::string DISP = "DISP";
                  return DISP;
               }
             const std::string& getLIST()
               {
                  static const std::string LIST = "LIST";
                  return LIST;
               }
             const std::string& getINFO()
               {
                  static const std::string INFO = "INFO";
                  return INFO;
               }
             const std::string& getICOP()
               {
                  static const std::string ICOP = "ICOP";
                  return ICOP;
               }
             const std::string& getISBJ()
               {
                  static const std::string ISBJ = "ISBJ";
                  return ISBJ;
               }
             const std::string& getICRD()
               {
                  static const std::string ICRD = "ICRD";
                  return ICRD;
               }
             const std::string& getISFT()
               {
                  static const std::string ISFT = "ISFT";
                  return ISFT;
               }
             const std::string& getAfsp()
               {
                  static const std::string afsp = "afsp";
                  return afsp;
               }
             const std::string& getICMT()
               {
                  static const std::string ICMT = "ICMT";
                  return ICMT;
               }
          } // namespace
        
        WavReader::WavReader(const std::string& filename)
          : _fileBuffer(mods::utils::FileUtils::mapFile(filename))
            {
               std::stringstream description;
               
               auto headerBuffer = _fileBuffer.slice<ChunkHeader>(0, 1);
               
               checkInit(headerBuffer->getChunkID() == getRIFF(), "Not a RIFF file");
               checkInit(_fileBuffer.size() >= headerBuffer->getChunkSize() - sizeof(ChunkHeader), "RIFF chunk not complete");
               
               auto riffHeader = _fileBuffer.slice<RiffHeader>(0, 1);
               
               checkInit(riffHeader->getFormat() == getWAVE(), "Not a WAVE file");
               
               auto riffBuffer = _fileBuffer.slice<u8>(sizeof(RiffHeader), riffHeader->chunk.getChunkSize() - sizeof(RiffHeader) + sizeof(ChunkHeader));
               
               using mods::utils::RBuffer;
               optional<Format> optFmt;
               optional<RBuffer<FactHeader>> optFactHeader;
               optional<RBuffer<u8>> optData;
               
               size_t offset = 0;
               while(offset < riffBuffer.size())
                 {
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
                         checkInit(chunkHeader->getChunkSize() <= riffBuffer.size() - offset - sizeof(ChunkHeader) &&
                                   chunkHeader->getChunkSize() >= sizeof(ListHeader) - sizeof(ChunkHeader) , "Incomplete LIST chunk");
                         
                         auto listHeader = riffBuffer.slice<ListHeader>(offset, 1);
                         
                         if(listHeader->getListTypeID() == getINFO())
                           {
                              parseInfoList(listHeader, riffBuffer, offset, description);
                           }
                         else
                           {
                              std::stringstream ss;
                              ss << "Unknown LIST chunk: " << listHeader->getListTypeID();
                              checkInit(false, ss.str());
                           }
                      }
                    else if(chunkHeader->getChunkID() == getAfsp())
                      {
                         readAfsp(chunkHeader, riffBuffer, offset, description);
                      }
                    else
                      {
                         std::stringstream ss;
                         ss << "Unknown RIFF chunk: " << chunkHeader->getChunkID();
                         checkInit(false, ss.str());
                      }
                    
                    auto chunkSize = chunkHeader->getChunkSize();
                    if((chunkSize & 1u) != 0) // padding
                      {
                         ++chunkSize;
                      }
                    offset += chunkSize + sizeof(ChunkHeader);
                 }
               
               checkInit(optFmt.has_value() && optData.has_value(), "Wav file should have at least a fmt and data chunk");
               
               auto& fmt = *optFmt;
               auto& data = *optData;
               _statCollector = std::make_shared<StatCollector>();
               _converter = WavConverter::buildConverter(data, fmt.getBitsPerSample(), fmt.getNumChannels(), fmt.getSampleRate(), _statCollector, fmt.getAudioFormat());
               _length = data.size();
               
               buildInfo(fmt.getBitsPerSample(), fmt.getNumChannels(), fmt.getSampleRate(), description.str(), fmt.getAudioFormat());
            }
        
        Format WavReader::readFMT(const mods::utils::RBuffer<ChunkHeader>& chunkHeader,
                                  const mods::utils::RBuffer<u8>& riffBuffer,
                                  size_t offset) const
          {
             checkInit(chunkHeader->getChunkSize() <= riffBuffer.size() - offset - sizeof(ChunkHeader) &&
                       chunkHeader->getChunkSize() >= sizeof(FmtHeader) - sizeof(ChunkHeader) , "Incomplete FMT chunk");
             
             auto fmtHeader = riffBuffer.slice<FmtHeader>(offset, 1);
             
             optional<mods::utils::RBuffer<ExtendedFmtHeader>> extendedFmtHeader;
             
             if(fmtHeader->chunk.getChunkSize() >= sizeof(ExtendedFmtHeader) - sizeof(ChunkHeader))
               {
                  mods::utils::RBuffer<ExtendedFmtHeader> buf = riffBuffer.slice<ExtendedFmtHeader>(offset, 1);
                  extendedFmtHeader = buf;
               }
             
             optional<mods::utils::RBuffer<ExtensibleHeader>> extensibleHeader;
             
             if(extendedFmtHeader.has_value() &&
                fmtHeader->chunk.getChunkSize() >= sizeof(ExtensibleHeader) - sizeof(ChunkHeader) &&
                (*extendedFmtHeader)->getExtensionSize() == (sizeof(ExtensibleHeader) - sizeof(ExtendedFmtHeader)))
               {
                  mods::utils::RBuffer<ExtensibleHeader> buf = riffBuffer.slice<ExtensibleHeader>(offset, 1);
                  extensibleHeader = buf;
               }
             
             auto format = fmtHeader->getAudioFormat();
             if(format == WavAudioFormat::EXTENSIBLE)
               {
                  if(extensibleHeader.has_value())
                    {
                       auto& ext = *extensibleHeader;
                       format = ext->getAudioFormat();
                       
                       if(!((fmtHeader->getNumChannels() == 1 && ext->getChannelMask() == 1) || (fmtHeader->getNumChannels() == 2 && ext->getChannelMask() == 3)))
                         {
                            checkInit(false, "Channel mask not supported yet");
                         }
                       checkInit(ext->getValidBitsPerSample() == ext->extendedFmt.fmt.getBitsPerSample(), "Ignoring bits in sample is not supported yet");
                    }
               }
             
             switch(format) 
               {
                case WavAudioFormat::PCM:
                  break;
                  
                case WavAudioFormat::A_LAW:
                    {
                       checkInit(fmtHeader->getBitsPerSample() == 8, "A-Law codec needs 8 bits per sample");
                       checkInit(extendedFmtHeader.has_value(), "A-law codec without extended fmt");
                    }
                  break;
                  
                case WavAudioFormat::MU_LAW:
                    {
                       checkInit(fmtHeader->getBitsPerSample() == 8, "MU-Law codec needs 8 bits per sample");
                       checkInit(extendedFmtHeader.has_value(), "MU-law codec without extended fmt");
                    }
                  break;
                  
                default:
                    {
                       std::stringstream ss;
                       ss << "Codec not supported: " << fmtHeader->getAudioFormatAsNumber();
                       checkInit(false, ss.str());
                    }
               }
             
             return Format(std::move(fmtHeader), std::move(extensibleHeader));
          }
        
        mods::utils::RBuffer<FactHeader> WavReader::readFact(const mods::utils::RBuffer<ChunkHeader>& chunkHeader,
                                                             const mods::utils::RBuffer<u8>& riffBuffer,
                                                             size_t offset) const
          {
             checkInit(chunkHeader->getChunkSize() <= riffBuffer.size() - offset - sizeof(ChunkHeader) &&
                       chunkHeader->getChunkSize() == sizeof(FactHeader) - sizeof(ChunkHeader) , "Incomplete Fact chunk");
             
             auto factHeader = riffBuffer.slice<FactHeader>(offset, 1);
             
             return factHeader;
          }
        
        mods::utils::RBuffer<u8> WavReader::readData(const mods::utils::RBuffer<ChunkHeader>& chunkHeader,
                                                     const mods::utils::RBuffer<u8>& riffBuffer,
                                                     size_t offset) const
          {
             checkInit(chunkHeader->getChunkSize() <= riffBuffer.size() - offset - sizeof(ChunkHeader),
                       "Incomplete Data chunk");
             
             auto data = riffBuffer.slice<u8>(offset + sizeof(ChunkHeader), chunkHeader->getChunkSize());
             
             return data;
          }
        
        void WavReader::readDisp(const mods::utils::RBuffer<ChunkHeader>& chunkHeader,
                                 const mods::utils::RBuffer<u8>& riffBuffer,
                                 size_t offset,
                                 std::stringstream& description) const
          {
             checkInit(chunkHeader->getChunkSize() <= riffBuffer.size() - offset - sizeof(ChunkHeader) &&
                       chunkHeader->getChunkSize() >= sizeof(DispHeader), "Incomplete Disp chunk");
             
             auto disp = riffBuffer.slice<DispHeader>(offset, 1);
             
             if(disp->getType() == DispType::TEXT)
               {
                  auto payload = riffBuffer.slice<u8>(offset + sizeof(DispHeader), chunkHeader->getChunkSize() - (sizeof(DispHeader) - sizeof(ChunkHeader)));
                  
                  if(description.str().length() > 0)
                    {
                       description << std::endl;
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
        
        void WavReader::readAfsp(const mods::utils::RBuffer<ChunkHeader>& chunkHeader,
                                 const mods::utils::RBuffer<u8>& riffBuffer,
                                 size_t offset,
                                 std::stringstream& description) const
          {
             checkInit(chunkHeader->getChunkSize() <= riffBuffer.size() - offset - sizeof(ChunkHeader) &&
                       chunkHeader->getChunkSize() >= sizeof(AfspHeader), "Incomplete Afsp chunk");
             
             auto stringBuffer = riffBuffer.slice<char>(offset + sizeof(AfspHeader), chunkHeader->getChunkSize() - (sizeof(AfspHeader) - sizeof(ChunkHeader)));
             std::string s = std::string(stringBuffer.begin(), stringBuffer.end());
             for(size_t i = 0; i < s.length(); ++i)
               {
                  if(s[i] == '\0' && i != s.length()-1)
                    {
                       s[i] = '\n';
                    }
               }
             if(!description.str().empty())
               {
                  description << std::endl;
               }
             description << "Afsp infos:" << s;
          }
        
        void WavReader::parseInfoList(const mods::utils::RBuffer<ListHeader>& listHeader,
                                      const mods::utils::RBuffer<u8>& riffBuffer,
                                      size_t offset,
                                      std::stringstream& description) const
          {
             auto listBuffer = riffBuffer.slice<u8>(offset + sizeof(ListHeader), listHeader->chunk.getChunkSize() - (sizeof(ListHeader) - sizeof(ChunkHeader)));
             size_t listOffset = 0;
             while(listOffset < listBuffer.size())
               {
                  auto chunkHeader = listBuffer.slice<ChunkHeader>(listOffset, 1);
                  auto stringBuffer = listBuffer.slice<char>(listOffset + sizeof(ChunkHeader), chunkHeader->getChunkSize());
                  std::string info = std::string(stringBuffer.begin(), stringBuffer.end());
                  if(!description.str().empty())
                    {
                       description << std::endl;
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
                  else
                    {
                       std::stringstream ss;
                       ss << "Unknown Info chunk: " << chunkHeader->getChunkID();
                       checkInit(false, ss.str());
                    }
                  description << info;
                  
                  auto chunkSize = chunkHeader->getChunkSize();
                  if((chunkSize & 1u) != 0) // padding
                    {
                       ++chunkSize;
                    }
                  listOffset += chunkSize + sizeof(ChunkHeader);
               }
          }
        
        void WavReader::buildInfo(int bitsPerSample, int nbChannels, int frequency, const std::string& description, WavAudioFormat codec)
          {
             std::stringstream ss;
             if(description.length() > 0)
               {
                  ss << "description: " << description << std::endl;
               }
             ss << "bits per sample: " << bitsPerSample << std::endl;
             ss << "number of channels: " << nbChannels << std::endl;
             ss << "frequency: " << frequency << std::endl;
             ss << "codec: " << toString(codec);
             _info = ss.str();
          }
        
        bool WavReader::isFinished() const
          {
             return _converter->isFinished();
          }
        
        void WavReader::read(mods::utils::RWBuffer<u8>* buf, int len)
          {
             _converter->read(buf, len);
          }
        
        std::string WavReader::getInfo() const
          {
             return _info;
          }
        
        std::string WavReader::getProgressInfo() const
          {
             size_t read = _statCollector->getBytesRead();
             size_t percentage = read * 100 / _length;
             std::stringstream ss;
             ss << "Reading..." << percentage << "%";
             return ss.str();
          }
     } // namespace wav
} // namespace mods
