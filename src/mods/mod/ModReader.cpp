
#include "mods/mod/ModReader.hpp"
#include "mods/utils/FileUtils.hpp"

#include <sstream>

namespace mods
{
   namespace mod
     {
        ModReader::ModReader(const std::string& fileName)
          : _fileBuffer(mods::utils::FileUtils::mapFile(fileName)),
          _notParsedBuffer(_fileBuffer.slice<u8>(0, _fileBuffer.size())),
          _songTitle(initializeSongTitle()),
          _numberOfInstruments(detectNumberOfInstruments()),
          _instruments(initializeInstruments())
            {
            }
        
        auto ModReader::initializeSongTitle() -> std::string
          {
             using SongArray = mods::utils::PackedArray<char, _songFieldLength>;
             
             checkInit(_fileBuffer.size() > _songFieldLength, "File too small to contain song title");
             auto buf = _fileBuffer.slice<SongArray>(0, 1);
             auto& array = buf[0];
             size_t length = 0;
             for(length = 0; length<_songFieldLength; ++length) 
               {
                  if(array[length] == '\0')
                    {
                       break;
                    }
               }
             
             _notParsedBuffer = _notParsedBuffer.slice<u8>(_songFieldLength, _notParsedBuffer.size() - _songFieldLength);
             return std::string(array.data(), length);
          }
        
        auto ModReader::detectNumberOfInstruments() const -> u32
          {
             constexpr static int oldModNumberOfInstruments = 16;
             constexpr static int newModNumberOfInstruments = 32;
             
             constexpr static int minValidAscii = 0x20;
             constexpr static int maxValidAscii = 0x7E;
             
             checkInit(_notParsedBuffer.size() >= oldModNumberOfInstruments * sizeof(Instrument),
                       "File is too small for 16 instruments");
             
             if(_notParsedBuffer.size() < newModNumberOfInstruments * sizeof(Instrument))
               {
                  return newModNumberOfInstruments;
               }
             
             auto tmp = _notParsedBuffer.slice<Instrument>(0, newModNumberOfInstruments);
             
             for(auto& instrument : tmp) 
               {
                  bool ascii = true;
                  for(auto& c : instrument.getSampleName())
                    {
                       if(c == '\0')
                         {
                            break;
                         }
                       if(c < minValidAscii && c > maxValidAscii)
                         {
                            ascii = false;
                            break;
                         }
                    }
                  if(!ascii)
                    {
                       return oldModNumberOfInstruments;
                    }
               }
             
             return newModNumberOfInstruments;
          }
        
        auto ModReader::initializeInstruments() -> mods::utils::RBuffer<Instrument>
          {
             auto instruments = _fileBuffer.slice<Instrument>(_songFieldLength, _numberOfInstruments);
             _notParsedBuffer = _notParsedBuffer.slice<u8>(_numberOfInstruments * sizeof(Instrument),
                                                           _notParsedBuffer.size() - _numberOfInstruments * sizeof(Instrument));
             return instruments;
          }
        
        auto ModReader::isFinished() const -> bool
          {
             std::cout << "TODO: ModReader::isFinished() const" << std::endl;
             return false;
          }
        
        void ModReader::read(mods::utils::RWBuffer<u8>* buf, int len)
          {
             std::cout << "TODO: ModReader::read(mods::utils::RWBuffer<u8>*, int)" << std::endl;
          }
        
        auto ModReader::getInfo() const -> std::string
          {
             std::stringstream ss;
             ss << getSongTitle() << std::endl;
             for(auto& instrument : _instruments)
               {
                  ss << instrument.getSampleName() << std::endl;
               }
             return ss.str();
             
             std::cout << "TODO: ModReader::getInfo() const" << std::endl;
             return "";
          }
        
        auto ModReader::getProgressInfo() const -> std::string
          {
             std::cout << "TODO: ModReader::getProgressInfo() const" << std::endl;
             return "";
          }
        
        auto ModReader::getSongTitle() const -> std::string
          {
             return _songTitle;
          }
     } // namespace mod
} // namespace mods
