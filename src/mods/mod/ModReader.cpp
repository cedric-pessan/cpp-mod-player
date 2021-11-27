
#include "mods/mod/ModReader.hpp"

namespace mods
{
   namespace mod
     {
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
             std::cout << "TODO: ModReader::getInfo() const" << std::endl;
             return "";
          }
        
        auto ModReader::getProgressInfo() const -> std::string
          {
             std::cout << "TODO: ModReader::getProgressInfo() const" << std::endl;
             return "";
          }
     } // namespace mod
} // namespace mods
