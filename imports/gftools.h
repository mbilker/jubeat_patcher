#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

size_t __declspec(dllimport) GFSLPrintf(void *a1, void *a2, const char *fmt, ...);
bool __declspec(dllimport) GFDbgIsEnableExitFunc();
void __declspec(dllimport) GFInfiniteLoop();
size_t __declspec(dllimport) GFReportPrefixedPrintf(
    int a1,
    const char* file,
    int line,
    const char *func,
    const char *level,
    const char *fmt,
    ...
);

int __declspec(dllimport) XFileLoadCall(const char *path, int mode, void *buf, size_t sz);
bool __declspec(dllimport) XFileIsBusy(int f);
int __declspec(dllimport) XFileLoadFinish(int f);

#define GFAssert(cond) if(!(cond)) { \
    GFReportPrefixedPrintf( \
        1, \
        __FILE__, \
        __LINE__, \
        __func__, \
        "F:", \
        "assertion failed '" #cond "'" \
    ); \
    if(GFDbgIsEnableExitFunc()) \
        GFInfiniteLoop(); \
}

#ifdef __cplusplus
};
#endif
