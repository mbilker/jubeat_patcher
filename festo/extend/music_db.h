#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

bool __cdecl music_db_get_sequence_filename_hook(void *a1, void *a2, int music_id, uint8_t seq);
bool __cdecl music_db_get_sound_filename_hook(void *a1, void *a2, int music_id, uint8_t seq);

#ifdef __cplusplus
};
#endif
