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

struct music_db_entry_t {
    int music_id;
    int parent_id;
    int name_sort_id_j;
    float detail_level_bsc;
    float detail_level_adv;
    float detail_level_ext;
    float bpm_max;
    float bpm_min;
    int music_type;
    int version;
    int16_t pos_index;
    int index_start;
    int is_default;         // was: char
    int is_card_default;    // was: char
    int is_offline_default; // was: char
    int is_hold;            // was: char
    int step;
    // must be contiguous as it's used as an array
    // pops/anime/socialmusic/game/classical/original/toho
    char genre_list[7];
    uint64_t grouping_category;
    uint8_t ultimate_list_omnimix;
    uint8_t ultimate_list_jubeat_plus;
    uint8_t ultimate_list_jubeat_2020;
    uint8_t ultimate_list_jukebeat;
    int32_t pack_id; // custom! Extend pack ID from Jubeat mobile versions
    // custom! real, human-readable name. Max seen was 64 bytes, 256 for massive
    // overkill in case of weird songs. Used to auto-generate sort IDs and sort
    // by name
    char title_name[256];
    char title_yomigana[256];
};

// useful when creating custom sort functions
music_db_entry_t *music_from_id(int id);

#ifdef __cplusplus
};
#endif
