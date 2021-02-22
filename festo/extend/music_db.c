//#define LOG_MODULE "extend::music_db"

#include <stdint.h>
#include <stdlib.h>

#include "imports/gftools.h"

#include "pe/iat.h"

//#include "util/log.h"

static bool __cdecl music_db_get_sequence_filename_hook(
    void *a1, void *a2, int music_id, uint8_t seq);
static bool __cdecl music_db_get_sound_filename_hook(void *a1, void *a2, int music_id, uint8_t seq);

static const struct hook_symbol jubeat_music_db_hooks[] = {
    {
        .name = "music_db_get_sequence_filename",
        .patch = (void *) music_db_get_sequence_filename_hook,
        .link = NULL,
    },
    {
        .name = "music_db_get_sound_filename",
        .patch = (void *) music_db_get_sound_filename_hook,
        .link = NULL,
    },
};

void hook_music_db(HANDLE process, HMODULE jubeat_handle)
{
    iat_hook_table_apply(
        process,
        jubeat_handle,
        "music_db.dll",
        jubeat_music_db_hooks,
        _countof(jubeat_music_db_hooks));
}

static bool __cdecl music_db_get_sequence_filename_hook(
    void *a1, void *a2, int music_id, uint8_t seq)
{
    //log_misc("music_db_get_sequence_filename(%p, %p, %d, %d)", a1, a2, music_id, seq);

    const char *seq_filename;

    if (seq == 1) {
        seq_filename = "adv";
    } else if (seq == 2) {
        seq_filename = "ext";
    } else {
        seq_filename = "bsc";
    }

    return GFSLPrintf(a1, a2, "%s/%09d/%s.eve", "data/music", music_id, seq_filename) >= 0;
}

static bool __cdecl music_db_get_sound_filename_hook(void *a1, void *a2, int music_id, uint8_t seq)
{
    //log_misc("music_db_get_sound_filename(%p, %p, %d, %d)", a1, a2, music_id, seq);

    bool use_idx = music_id != 90010017 && seq > 0;

    return GFSLPrintf(a1, a2, "%s/%09d/%s.bin", "data/music", music_id, use_idx ? "idx" : "bgm") >=
           0;
}
