#include "festo.h"

#include "util/patch.h"

const struct patch_t song_unlock_patch {
    .name = "song unlock patch",
    .pattern = { 0x74, 0x0A, 0x8B, 0x4C },
    .data = { 0x90, 0x90 },
    .data_offset = 0,
};

void festo_apply_common_patches(HANDLE process, const MODULEINFO &jubeat_info, const MODULEINFO &music_db_info)
{
    do_patch(process, music_db_info, song_unlock_patch);
}
