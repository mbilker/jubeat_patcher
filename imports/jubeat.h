#pragma once

#include <stdbool.h>

#include "util/defs.h"

#ifdef __cplusplus
extern "C" {
#endif

DLL_IMPORT bool dll_entry_init(char *, void *);
DLL_IMPORT bool dll_entry_main(void);

#ifdef __cplusplus
};
#endif
