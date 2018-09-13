
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
          } // namespace
        
        WavReader::WavReader(const std::string& filename)
          : _fileBuffer(mods::utils::FileUtils::mapFile(filename)),
          _headerBuffer(_fileBuffer.slice<ChunkHeader>(0, 1))
            {
               checkInit(_headerBuffer->getChunkID() == getRIFF(), "Not a RIFF file");
               checkInit(_fileBuffer.size() >= _headerBuffer->getChunkSize() - sizeof(ChunkHeader), "RIFF chunk not complete");
               
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
                    else
                      {
                         std::stringstream ss;
                         ss << "Unknown RIFF chunk: " << chunkHeader->getChunkID();
                         checkInit(false, ss.str());
                      }
                    
                    offset += chunkHeader->getChunkSize() + sizeof(ChunkHeader);
                 }
               
               checkInit(optFmtHeader.has_value() && optData.has_value(), "Wav file should have at least a fmt and data chunk");
               
               auto& fmtHeader = *optFmtHeader;
               _converter = WavConverter::buildConverter(fmtHeader->getBitsPerSample(), fmtHeader->getNumChannels());
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
                       "Incomplete Fact chunk");
             
             auto data = riffBuffer.slice<u8>(offset + sizeof(ChunkHeader), chunkHeader->getChunkSize());
             
             return data;
          }
        
        bool WavReader::isFinished() const
          {
             return _converter->isFinished();
          }
        
     } // namespace wav
} // namespace mods
