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
   auto modsOpen(const char* pathName, int flags) -> int;
   auto modsHasMapFailed(void* ptr) -> int;
}
#endif

#endif // MODS_UTILS_IMPL_UNIX_FILEUTILS_H