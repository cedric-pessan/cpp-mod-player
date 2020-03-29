
#include "mods/utils/impl/unix/FileUtils.h"

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

int modsOpen(const char* pathName, int flags)
{
   return open(pathName, flags);
}

int modsHasMapFailed(void* ptr)
{
   if(ptr == MAP_FAILED)
     {
        return FILEUTILS_TRUE;
     }
   return FILEUTILS_FALSE;
}
