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

enum genre_type: uint8_t {
    GENRE_NOT_IN = 0,
    // the song is in this genre
    GENRE_SECONDARY = 1,
    // the song is in this genre, and when grouping songs for display, it should
    // be considered the "main" genre
    GENRE_PRIMARY = 2,
};

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
    union {
        uint8_t genre_list[7];
        // names for convenience
        struct __attribute__((__packed__)) {
            enum genre_type genre_pops;
            enum genre_type genre_anime;
            enum genre_type genre_socialmusic;
            enum genre_type genre_game;
            enum genre_type genre_classical;
            enum genre_type genre_original;
            enum genre_type genre_toho;
        };
    };
    uint64_t grouping_category;
    uint8_t ultimate_list_vanilla;
    uint8_t ultimate_list_omnimix;
    uint8_t ultimate_list_jubeat_plus;
    uint8_t ultimate_list_jubeat_2020;
    uint8_t ultimate_list_jukebeat;
    int32_t pack_id; // custom! Extend pack ID from Jubeat mobile versions
    // custom! real, human-readable name. Max seen was 64 bytes, 256 for massive
    // overkill in case of weird songs. Used to auto-generate sort IDs and sort
    // by name
    char title_name[256];
    char sort_name[256];
};

// useful when creating custom sort functions
music_db_entry_t *music_from_id(int id);
bool __cdecl music_db_is_exists_version_from_ver1(int id);
bool __cdecl music_db_is_exists_version_from_ver2(int id);
bool __cdecl music_db_is_exists_version_from_ver3(int id);
bool __cdecl music_db_is_exists_version_from_ver4(int id);
bool __cdecl music_db_is_exists_version_from_ver5(int id);
bool __cdecl music_db_is_exists_version_from_ver5_5(int id);
bool __cdecl music_db_is_exists_version_from_ver6(int id);
bool __cdecl music_db_is_exists_version_from_ver7(int id);
bool __cdecl music_db_is_exists_version_from_ver8(int id);
bool __cdecl music_db_is_exists_version_from_ver9(int id);

#ifdef __cplusplus
};
#endif
