#pragma once

#include <stdbool.h>

#include "util/defs.h"

#ifdef __cplusplus
extern "C" {
#endif

DLL_IMPORT size_t GFSLPrintf(void *a1, void *a2, const char *fmt, ...);
DLL_IMPORT bool GFDbgIsEnableExitFunc(void);
DLL_IMPORT void GFInfiniteLoop(void);
DLL_IMPORT size_t GFReportPrefixedPrintf(
    int a1, const char *file, int line, const char *func, const char *level, const char *fmt, ...);

DLL_IMPORT int XFileLoadCall(const char *path, int mode, void *buf, size_t sz);
DLL_IMPORT bool XFileIsBusy(int f);
DLL_IMPORT int XFileLoadFinish(int f);

#define GFAssert(cond)                                                                             \
    if (!(cond)) {                                                                                 \
        GFReportPrefixedPrintf(                                                                    \
            1, __FILE__, __LINE__, __func__, "F:", "assertion failed '" #cond "'");                \
        if (GFDbgIsEnableExitFunc())                                                               \
            GFInfiniteLoop();                                                                      \
    }

#ifdef __cplusplus
};
#endif
