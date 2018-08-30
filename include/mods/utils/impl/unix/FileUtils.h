#ifndef MODS_UTILS_IMPL_UNIX_FILEUTILS_H
#define MODS_UTILS_IMPL_UNIX_FILEUTILS_H

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

#endif // MODS_UTILS_IMPL_UNIX_FILEUTILS_H