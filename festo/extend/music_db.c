//#define LOG_MODULE "extend::music_db"

#include <stdint.h>

#include "imports/gftools.h"

//#include "util/log.h"

bool __cdecl music_db_get_sequence_filename_hook(void *a1, void *a2, int music_id, uint8_t seq) {
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

bool __cdecl music_db_get_sound_filename_hook(void *a1, void *a2, int music_id, uint8_t seq) {
    //log_misc("music_db_get_sound_filename(%p, %p, %d, %d)", a1, a2, music_id, seq);

    bool use_idx = music_id != 90010017 && seq > 0;

    return GFSLPrintf(a1, a2, "%s/%09d/%s.bin", "data/music", music_id, use_idx ? "idx" : "bgm") >= 0;
}
