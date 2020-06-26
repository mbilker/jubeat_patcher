//#define LOG_MODULE "extend::music_db"

#include <stdint.h>

#include "imports/avs2-core/avs.h"
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

void *__cdecl music_db_avs_property_mem_read(
        void *prop,
        void *prop_node,
        unsigned int flags,
        uint8_t *buffer,
        uint32_t buffer_size)
{
    //log_misc("property_mem_read(%p, %p, 0x%x, %p, %u)", prop, prop_node, flags, buffer, buffer_size);

    // Add long mode flag
    flags |= 0x1000u;

    return property_mem_read(prop, prop_node, flags, buffer, buffer_size);
}
