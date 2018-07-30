
#include "modopen.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

int modOpen(const char* pathName, int flags)
{
   return open(pathName, flags);
}
