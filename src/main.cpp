
#include <iostream>

int main(int argc, char** argv)
{
   if(argc < 2)
     {
        std::cout << "no input file" << std::endl;
        return 0;
     }
   
   std::string filename = argv[1];
   auto dotIdx = filename.find_last_of(".");
   if(dotIdx == std::string::npos) 
     {
        std::cout << "count not find extension: " << filename << std::endl;
        return 0;
     }
   std::string format = filename.substr(dotIdx+1);
   
   std::cout << "unknown format:" << format << std::endl;
   return 0;
}
