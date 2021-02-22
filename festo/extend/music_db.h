#pragma once

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

void hook_music_db(HANDLE process, HMODULE jubeat_handle);

#ifdef __cplusplus
};
#endif
