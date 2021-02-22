#ifndef UTIL_DEFS_H
#define UTIL_DEFS_H

#include <stdint.h>

#define STRINGIFY_(x) #x
#define STRINGIFY(x) STRINGIFY_(x)

#define DLL_EXPORT __declspec(dllexport)
#define DLL_IMPORT __declspec(dllimport)
#define STDCALL __stdcall

#define containerof(ptr, outer_t, member) ((void *) (((uint8_t *) ptr) - offsetof(outer_t, member)))

#endif
