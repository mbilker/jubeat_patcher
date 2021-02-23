#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// default game limit is 2048, ulti uses ~2400
#define MAX_SONGS 8192
#define MAX_SONGS_STOCK 2048
// bitfield
//#define FLAG_LEN (MAX_SONGS / 8)
// we cannot feasibly replace every set of flags, so just use the default one
#define FLAG_LEN (MAX_SONGS_STOCK / 8)

void hook_music_db(HANDLE process, HMODULE jubeat_handle, HMODULE music_db_handle);

#ifdef __cplusplus
};
#endif
