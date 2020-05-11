#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

bool __declspec(dllimport) jb_dll_entry_init(char *, void *);
bool __declspec(dllimport) jb_dll_entry_main(void);

#ifdef __cplusplus
};
#endif
