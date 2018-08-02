#ifndef _FILEUTILS_H_
#define _FILEUTILS_H_

enum
{
   FILEUTILS_FALSE,
   FILEUTILS_TRUE
};

#ifdef __cplusplus
extern "C"
{
#endif
   int modsOpen(const char* pathName, int flags);
   int modsHasMapFailed(void* ptr);
#ifdef __cplusplus
}
#endif

#endif // _FILEUTILS_H_