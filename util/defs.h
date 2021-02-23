#ifndef UTIL_DEFS_H
#define UTIL_DEFS_H

#include <stdint.h>

#define STRINGIFY_(x) #x
#define STRINGIFY(x) STRINGIFY_(x)

// Exports are controlled via `.def` files in MSVC
#ifndef _MSC_VER
#define DLL_EXPORT __attribute__((visibility("default")))
#else
#define DLL_EXPORT
#endif

#define DLL_IMPORT __declspec(dllimport)

#define STDCALL __stdcall

#define containerof(ptr, outer_t, member) ((void *) (((uint8_t *) ptr) - offsetof(outer_t, member)))

#endif
