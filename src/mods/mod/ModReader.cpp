
#include "mods/mod/ModReader.hpp"
#include "mods/utils/FileUtils.hpp"

#include <sstream>

namespace mods
{
   namespace mod
     {
        ModReader::ModReader(const std::string& fileName)
          : _fileBuffer(mods::utils::FileUtils::mapFile(fileName)),
          _songTitle(initializeSongTitle()),
          _numberOfInstruments(detectNumberOfInstruments()),
          _instruments(initializeInstruments())
            {
             /*check16;
             check32; -> checktitles;
             _instruments = _instrumentRawBuffer.slice<Instrument>(s_songTitleLength, s_legacyNumberOfInstruments);
             checkinstrumentTitles;*/
            }
        
        auto ModReader::initializeSongTitle() const -> std::string
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
             
             return std::string(array.data(), length);
          }
        
        auto ModReader::detectNumberOfInstruments() const -> u32
          {
             std::cout << "TODO: ModReader::detectNumberOfInstruments() const" << std::endl;
             return 0;
          }
        
        auto ModReader::initializeInstruments() const -> mods::utils::RBuffer<Instrument>
          {
             return _fileBuffer.slice<Instrument>(_songFieldLength, _numberOfInstruments);
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
