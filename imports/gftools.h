#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

size_t __declspec(dllimport) GFSLPrintf(void *a1, void *a2, const char *fmt, ...);

#ifdef __cplusplus
};
#endif
