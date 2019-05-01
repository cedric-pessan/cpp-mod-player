
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
               optional<RBuffer<FmtHeader>> optFmtHeader;
               optional<RBuffer<FactHeader>> optFactHeader;
               optional<RBuffer<u8>> optData;
               
               size_t offset = 0;
               while(offset < riffBuffer.size())
                 {
                    auto chunkHeader = riffBuffer.slice<ChunkHeader>(offset, 1);
                    
                    if(chunkHeader->getChunkID() == getFMT())
                      {
                         checkInit(!optFmtHeader.has_value(), "Multiple fmt chunks defined");
                         optFmtHeader = readFMT(chunkHeader, riffBuffer, offset);
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
                              parseInfoList(listHeader, riffBuffer, offset);
                           }
                         else
                           {
                              std::stringstream ss;
                              ss << "Unknown LIST chunk: " << listHeader->getListTypeID();
                              checkInit(false, ss.str());
                           }
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
               
               checkInit(optFmtHeader.has_value() && optData.has_value(), "Wav file should have at least a fmt and data chunk");
               
               auto& fmtHeader = *optFmtHeader;
               auto& data = *optData;
               _statCollector = std::make_shared<StatCollector>();
               _converter = WavConverter::buildConverter(data, fmtHeader->getBitsPerSample(), fmtHeader->getNumChannels(), fmtHeader->getSampleRate(), _statCollector);
               _length = data.size();
               
               buildInfo(fmtHeader->getBitsPerSample(), fmtHeader->getNumChannels(), fmtHeader->getSampleRate(), description.str());
            }
        
        mods::utils::RBuffer<FmtHeader> WavReader::readFMT(const mods::utils::RBuffer<ChunkHeader>& chunkHeader,
                                                           const mods::utils::RBuffer<u8>& riffBuffer,
                                                           size_t offset) const
          {
             checkInit(chunkHeader->getChunkSize() <= riffBuffer.size() - offset - sizeof(ChunkHeader) &&
                       chunkHeader->getChunkSize() >= sizeof(FmtHeader) - sizeof(ChunkHeader) , "Incomplete FMT chunk");
             
             auto fmtHeader = riffBuffer.slice<FmtHeader>(offset, 1);
             
             checkInit(fmtHeader->getAudioFormat() == WavAudioFormat::PCM, "Only PCM is supported at the moment");
             
             checkInit(fmtHeader->chunk.getChunkSize() == sizeof(FmtHeader) - sizeof(ChunkHeader), "Extra fmt infos not yet implemented");
             
             return fmtHeader;
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
        
        void WavReader::parseInfoList(const mods::utils::RBuffer<ListHeader>& listHeader,
                                      const mods::utils::RBuffer<u8>& riffBuffer,
                                      size_t offset) const
          {
             auto listBuffer = riffBuffer.slice<u8>(offset + sizeof(ListHeader), listHeader->chunk.getChunkSize() - (sizeof(ListHeader) - sizeof(ChunkHeader)));
             size_t listOffset = 0;
             while(listOffset < listBuffer.size())
               {
                  auto chunkHeader = listBuffer.slice<ChunkHeader>(listOffset, 1);
                  
                    {
                       std::stringstream ss;
                       ss << "Unknown Info chunk: " << chunkHeader->getChunkID();
                       checkInit(false, ss.str());
                    }
                  
                  auto chunkSize = chunkHeader->getChunkSize();
                  if((chunkSize & 1u) != 0) // padding
                    {
                       ++chunkSize;
                    }
                  listOffset += chunkSize + sizeof(ChunkHeader);
               }
          }
        
        void WavReader::buildInfo(int bitsPerSample, int nbChannels, int frequency, const std::string& description)
          {
             std::stringstream ss;
             if(description.length() > 0)
               {
                  ss << "description: " << description << std::endl;
               }
             ss << "bits per sample: " << bitsPerSample << std::endl;
             ss << "number of channels: " << nbChannels << std::endl;
             ss << "frequency: " << frequency;
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
