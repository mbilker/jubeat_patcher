//#define LOG_MODULE "extend::music_db"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <windows.h>

#include "imports/avs2-core/avs.h"
#include "imports/gftools.h"

#include "util/robin_hood.h"

#include "music_db.h"

//#include "util/log.h"

bool __cdecl (*music_db_initialize_orig)();

// default: 2meg, ultimate: 6meg
#define MDB_XML_SIZE (6 * 1024 * 1024)

bool __cdecl music_db_get_sequence_filename(void *a1, void *a2, int music_id, uint8_t seq) {
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

bool __cdecl music_db_get_sound_filename(void *a1, void *a2, int music_id, uint8_t seq) {
    //log_misc("music_db_get_sound_filename(%p, %p, %d, %d)", a1, a2, music_id, seq);

    bool use_idx = music_id != 90010017 && seq > 0;

    return GFSLPrintf(a1, a2, "%s/%09d/%s.bin", "data/music", music_id, use_idx ? "idx" : "bgm") >= 0;
}

// d3_initialize calls this at its very end, so we use the lucky uniqueness
// of the size parameter to overwrite the texture memory + texture limit
// after it's called
void __cdecl *mem_set(void *s, int c, size_t n) {
    if(n == 1296) {
        log_body_info("ultimate", "hooked d3_initialize");

        uint16_t *limit = (uint16_t*)((DWORD)s + 0xAEE8);
        void **buf = (void**)((DWORD)s - 0x6F3260);

        const size_t new_sz = 8192 * 28 * 4;
        size_t new_limit = new_sz / 28;
        *buf = malloc(new_sz);
        *limit = new_limit;
    }

    return memset(s, c, n);
}

typedef struct {
  int music_id;
  int parent_id;
  int name_sort_id_j;
  char name_string[64];
  uint8_t level_bsc;
  uint8_t level_adv;
  uint8_t level_ext;
  float detail_level_bsc;
  float detail_level_adv;
  float detail_level_ext;
  float bpm_max;
  float bpm_min;
  int music_type;
  int version;
  int16_t pos_index;
  int index_start;
  int is_default; // was: char
  int is_card_default; // was: char
  int is_offline_default; // was: char
  int is_hold; // was: char
  int step;
  // must be contiguous as it's used as an array
  // pops/anime/socialmusic/game/classical/original/toho
  char genre_list[7];
  uint64_t grouping_category;
  int32_t pack_id; // custom! Extend pack ID from Jubeat mobile versions
} music_db_entry_t;

static int music_count;
static music_db_entry_t music_db[MAX_SONGS];
static robin_hood::unordered_map<int, music_db_entry_t*> music_db_map;

void debug_music_entry(music_db_entry_t *song) {
    if(!song) {
        log_body_warning("ultimate", "%s: song == NULL", __func__);
        return;
    }
    log_body_misc("ultimate", "music_id = %d", song->music_id);
    log_body_misc("ultimate", "parent_id = %d", song->parent_id);
    log_body_misc("ultimate", "name_sort_id_j = %x", song->name_sort_id_j);
    log_body_misc("ultimate", "name_string = %s", song->name_string);
    log_body_misc("ultimate", "level_bsc = %d", (int)song->level_bsc);
    log_body_misc("ultimate", "level_adv = %d", (int)song->level_adv);
    log_body_misc("ultimate", "level_ext = %d", (int)song->level_ext);
    log_body_misc("ultimate", "detail_level_bsc = %f", song->detail_level_bsc);
    log_body_misc("ultimate", "detail_level_adv = %f", song->detail_level_adv);
    log_body_misc("ultimate", "detail_level_ext = %f", song->detail_level_ext);
    log_body_misc("ultimate", "bpm_max = %f", song->bpm_max);
    log_body_misc("ultimate", "bpm_min = %f", song->bpm_min);
    log_body_misc("ultimate", "music_type = %d", song->music_type);
    log_body_misc("ultimate", "version = %x", song->version);
    log_body_misc("ultimate", "pos_index = %d", (int)song->pos_index);
    log_body_misc("ultimate", "index_start = %d", song->index_start);
    log_body_misc("ultimate", "is_default = %d", song->is_default);
    log_body_misc("ultimate", "is_card_default = %d", song->is_card_default);
    log_body_misc("ultimate", "is_offline_default = %d", song->is_offline_default);
    log_body_misc("ultimate", "is_hold = %d", song->is_hold);
    log_body_misc("ultimate", "step = %d", song->step);
    log_body_misc("ultimate", "genre_list_pops = %d", song->genre_list[0]);
    log_body_misc("ultimate", "genre_list_anime = %d", song->genre_list[1]);
    log_body_misc("ultimate", "genre_list_socialmusic = %d", song->genre_list[2]);
    log_body_misc("ultimate", "genre_list_game = %d", song->genre_list[3]);
    log_body_misc("ultimate", "genre_list_classical = %d", song->genre_list[4]);
    log_body_misc("ultimate", "genre_list_original = %d", song->genre_list[5]);
    log_body_misc("ultimate", "genre_list_toho = %d", song->genre_list[6]);
    log_body_misc("ultimate", "grouping_category = %lX", (long int)song->grouping_category);
    log_body_misc("ultimate", "pack_id = %d", song->pack_id);
}

enum music_load_res {
    MUSIC_LOAD_OK      = 1,
    MUSIC_LOAD_BAD_VER = 2,
    MUSIC_LOAD_FULL    = 3,
};

static music_db_entry_t* music_from_id(int id) {
    auto search = music_db_map.find(id);
    return search == music_db_map.end() ? NULL : search->second;
}

typedef bool (*music_filter_func)(music_db_entry_t *song);

bool filter_func_all(music_db_entry_t *song) {
    return true;
}

// the non-extend stuff falls below the 2048 song limit (about 1300 songs)
// this is thus a nice easy way to get the arcade tracks into jubility calcs
bool filter_func_not_extend(music_db_entry_t *song) {
    return song->pack_id == -1;
}

bool filter_func_is_default(music_db_entry_t *song) {
    return song->is_default;
}

bool filter_func_card_default(music_db_entry_t *song) {
    return song->is_card_default;
}

bool filter_func_is_offline_default(music_db_entry_t *song) {
    return song->is_offline_default;
}

static int music_db_filtered_list(const char *func, int limit, int *results, music_filter_func filter) {
    int returned = 0;
    int found = 0;

    for(int i = 0; i < music_count; i++) {
        music_db_entry_t *song = &music_db[i];

        if(!filter(song)) {
            continue;
        }

        found++;
        if(returned < limit) {
            returned++;
            *results++ = song->music_id;
        }
    }

    if(found != returned) {
        log_body_warning(
            "ultimate",
            "%s could have returned %d but capped at %d",
            func, found, returned);
    }

    log_body_misc("ultimate", "%s(%d, %p) -> %d", func, limit, results, returned);

    return returned;
}

int __cdecl music_db_get_default_list(int limit, int* results) {
    return music_db_filtered_list(__func__, limit, results, filter_func_not_extend);
    //return music_db_filtered_list(__func__, limit, results, filter_func_is_default);
}

int __cdecl music_db_get_offline_default_list(int limit, int* results) {
    return music_db_filtered_list(__func__, limit, results, filter_func_not_extend);
    //return music_db_filtered_list(__func__, limit, results, filter_func_is_offline_default);
}

// any song missing from here won't be included in jubility pick-up, and any
// song missing from here that blindly returns true in music_db_is_permitted
// will errorneously increment jubility locally when played, but reset next
// login. Patch the initial jubility function's arrays to allow full
// functionality
int __cdecl music_db_get_all_permitted_list(int limit, int *results) {
    return music_db_filtered_list(__func__, limit, results, filter_func_all);
}

int __cdecl music_db_get_possession_list(uint8_t flags[FLAG_LEN], int limit, int *results) {
    //log_body_warning("ultimate", "music_db_get_possession_list(%p, %d, %p)", flags, limit, results);

    return music_db_filtered_list(__func__, limit, results, filter_func_all);
}

int __cdecl music_db_get_card_default_list(int limit, int *results) {
    return music_db_filtered_list(__func__, limit, results, filter_func_card_default);
}

// int __cdecl music_db_get_jukebox_list() {
//     log_body_warning("ultimate", "music_db_get_jukebox_list: not implemented");
//     return 0;
// }

static enum music_load_res music_load_individual(int index, void *node) {
    // big enough for the hex name
    char tmp[256];

    if(index >= MAX_SONGS) {
        return MUSIC_LOAD_FULL;
    }

    music_db_entry_t *song = &music_db[index];

    property_node_refer(NULL, node, "/version", PROP_TYPE_str, tmp, sizeof(tmp));
    song->version = strtoul(tmp, NULL, 16);
    if (!song->version)
        return MUSIC_LOAD_BAD_VER;

    // sane defaults
    memset(song->name_string, 0, sizeof(song->name_string));
    memset(song->genre_list, 0, sizeof(song->genre_list));
    song->music_id = -1;
    song->parent_id = -1;
    song->name_sort_id_j = -1;
    song->level_bsc = 0;
    song->level_adv = 0;
    song->level_ext = 0;
    song->detail_level_bsc = 0;
    song->detail_level_adv = 0;
    song->detail_level_ext = 0;
    song->bpm_max = 0;
    song->bpm_min = 0;
    song->music_type = -1;
    song->pos_index = -1;
    song->index_start = -1;
    song->is_default = -1;
    song->is_card_default = -1;
    song->is_offline_default = -1;
    song->is_hold = -1;
    song->pack_id = -1;
    song->step = -1;
    song->grouping_category = -1;

    property_node_refer(NULL, node, "/music_id", PROP_TYPE_s32, &song->music_id, 4);
    property_node_refer(NULL, node, "/parent_id", PROP_TYPE_s32, &song->parent_id, 4);
    property_node_refer(NULL, node, "/bpm_max", PROP_TYPE_float, &song->bpm_max, 4);
    property_node_refer(NULL, node, "/bpm_min", PROP_TYPE_float, &song->bpm_min, 4);
    property_node_refer(NULL, node, "/name_sort_id_j", PROP_TYPE_str, tmp, sizeof(tmp));
    song->name_sort_id_j = strtoul(tmp, NULL, 16);
    property_node_refer(NULL, node, "/music_type", PROP_TYPE_s32, &song->music_type, 4);
    property_node_refer(NULL, node, "/level_bsc", PROP_TYPE_u8, &song->level_bsc, 1);
    property_node_refer(NULL, node, "/level_adv", PROP_TYPE_u8, &song->level_adv, 1);
    property_node_refer(NULL, node, "/level_ext", PROP_TYPE_u8, &song->level_ext, 1);
    if (property_node_refer(NULL, node, "/detail_level_bsc", PROP_TYPE_float, &song->detail_level_bsc, 4) < 0)
        song->detail_level_bsc = (float)song->level_bsc;
    if (property_node_refer(NULL, node, "/detail_level_adv", PROP_TYPE_float, &song->detail_level_adv, 4) < 0)
        song->detail_level_adv = (float)song->level_adv;
    if (property_node_refer(NULL, node, "/detail_level_ext", PROP_TYPE_float, &song->detail_level_ext, 4) < 0)
        song->detail_level_ext = (float)song->level_ext;
    property_node_refer(NULL, node, "/pos_index", PROP_TYPE_s16, &song->pos_index, 2);
    property_node_refer(NULL, node, "/is_default", PROP_TYPE_s32, &song->is_default, 4);
    property_node_refer(NULL, node, "/is_card_default", PROP_TYPE_s32, &song->is_card_default, 4);
    property_node_refer(NULL, node, "/is_offline_default", PROP_TYPE_s32, &song->is_offline_default, 4);
    property_node_refer(NULL, node, "/is_hold", PROP_TYPE_s32, &song->is_hold, 4);
    property_node_refer(NULL, node, "/index_start", PROP_TYPE_s32, &song->index_start, 4);
    property_node_refer(NULL, node, "/step", PROP_TYPE_s32, &song->step, 4);
    property_node_refer(NULL, node, "genre/pops", PROP_TYPE_u8, &song->genre_list[0], 1);
    property_node_refer(NULL, node, "genre/anime", PROP_TYPE_u8, &song->genre_list[1], 1);
    property_node_refer(NULL, node, "genre/socialmusic", PROP_TYPE_u8, &song->genre_list[2], 1);
    property_node_refer(NULL, node, "genre/game", PROP_TYPE_u8, &song->genre_list[3], 1);
    property_node_refer(NULL, node, "genre/classic", PROP_TYPE_u8, &song->genre_list[4], 1);
    property_node_refer(NULL, node, "genre/original", PROP_TYPE_u8, &song->genre_list[5], 1);
    property_node_refer(NULL, node, "genre/toho", PROP_TYPE_u8, &song->genre_list[6], 1);
    property_node_refer(NULL, node, "/pack_id", PROP_TYPE_s32, &song->pack_id, 4);
    property_node_refer(NULL, node, "/grouping_category", PROP_TYPE_str, tmp, sizeof(tmp));
    song->grouping_category = strtoul(tmp, NULL, 16);

    if (song->music_id == 70000154 && !song->grouping_category) {
        song->grouping_category = 4736;
    }

    // YOU HAVE A SHIFT-JIS XML FORMAT YOU IDIOTS
    property_node_refer(NULL, node, "/name_string", PROP_TYPE_str, tmp, sizeof(tmp));
    // clamp the 256 hex/128 real byte input to the 64 byte output
    tmp[(sizeof(song->name_string) - 1) * 2] = '\0';

    int i;
    for(i = 0; tmp[i]; i+= 2) {
        char blah[3];
        blah[0] = tmp[i];
        blah[1] = tmp[i+1];
        blah[2] = '\0';
        song->name_string[i/2] = strtoul(blah, NULL, 16);
    }
    song->name_string[i/2] = '\0';

    music_db_map[song->music_id] = song;

    return MUSIC_LOAD_OK;
}

bool __cdecl music_db_initialize() {
    // some features are not worth reimplementing because their results aren't
    // changed by the ultimate songs. This lets us use these functions easily.
    music_db_initialize_orig();

    log_body_info("ultimate", "music_db_initialize...");
    music_count = 0;
    memset(music_db, 0, sizeof(music_db));
    music_db_map.clear();

    void *prop = NULL;
    void *prop_mem = NULL;
    //void *mdb_mem = avs_gheap_allocate(0, MDB_XML_SIZE, 0);
    void *mdb_mem = malloc(MDB_XML_SIZE);
    // note: loading music_ulti.xml
    int f = XFileLoadCall("data/music_info/music_ulti.xml", 1, mdb_mem, MDB_XML_SIZE);
    if(f) {
        while(XFileIsBusy(f)) {
            avs_thread_delay(4, 0);
        }
        int r = XFileLoadFinish(f);
        GFAssert(r);

        //prop_mem = avs_gheap_allocate(0, MDB_XML_SIZE, 0);
        prop_mem = malloc(MDB_XML_SIZE);
        prop = property_mem_read(mdb_mem, r, 23 | 0x1000u, prop_mem, MDB_XML_SIZE);
    }

    free(mdb_mem);
    // if(mdb_mem) {
    //     avs_gheap_free(mdb_mem);
    // }

    GFAssert(prop != NULL);

    property_clear_error(prop);
    void *body = property_search(prop, 0, "/music_data/body");

    void *song = property_node_traversal(body, TRAVERSE_FIRST_CHILD);
    for(; song; song = property_node_traversal(song, TRAVERSE_NEXT_SIBLING)) {
        if(music_load_individual(music_count, song) == MUSIC_LOAD_OK)
            ++music_count;
    }

    if(prop) {
        property_destroy(prop);
        //avs_gheap_free(prop_mem);
    }
    free(prop_mem);

    log_body_info("ultimate", "Loaded %d songs into music db", music_count);

    return true;
}

// bool __cdecl music_db_finalize() {
//     return true;
// }

// bool __cdecl music_db_reset_using_datapackage(int a1) {
//     return true;
// }

// int __cdecl music_db_dbg_get_all_list() {
//     log_body_warning("ultimate", "music_db_dbg_get_all_list: not implemented");
//     return 0;
// }

// int __cdecl music_db_dot_array_to_music_bar() {
//     log_body_warning("ultimate", "music_db_dot_array_to_music_bar: not implemented");
//     return 0;
// }

float __cdecl music_db_get_bpm(int id) {
    music_db_entry_t *song = music_from_id(id);
    return song ? song->bpm_max : 0.0;
}

float __cdecl music_db_get_bpm_min(int id) {
    music_db_entry_t *song = music_from_id(id);
    return song ? song->bpm_min : -1.0;
}

// get_default_x: returns the song to play first? No patch needed

// int __cdecl music_db_get_default_id() {
//     log_body_warning("ultimate", "music_db_get_default_id: not implemented");
//     return 0;
// }

// int __cdecl music_db_get_default_id_by_genre() {
//     log_body_warning("ultimate", "music_db_get_default_id_by_genre: not implemented");
//     return 0;
// }

// int __cdecl music_db_get_default_id_by_mode() {
//     log_body_warning("ultimate", "music_db_get_default_id_by_mode: not implemented");
//     return 0;
// }

char* __cdecl music_db_get_genre_list(int id) {
    music_db_entry_t *song = music_from_id(id);
    GFAssert(song != NULL); // this will crash if it's NULL anyway, so may as well report cleanly

    return song->genre_list;
}

uint64_t __cdecl music_db_get_grouping_category_list(int id) {
    music_db_entry_t *song = music_from_id(id);
    return song ? song->grouping_category : 0;
}

int __cdecl music_db_get_index_start(int id) {
    music_db_entry_t *song = music_from_id(id);
    return song ? song->index_start : -1;
}


uint8_t __cdecl music_db_get_level(int id, uint8_t difficulty) {
    music_db_entry_t *song = music_from_id(id);
    if(!song) {
        return 1;
    }

    switch(difficulty) {
        case 0:
            return song->level_bsc;
        case 1:
            return song->level_adv;
        case 2:
            return song->level_ext;
        default:
            return 1;
    }
}

// we were having problems here, if they come back, uncomment and hook this
// uint8_t __cdecl music_db_get_level_logged(int id, uint8_t difficulty) {
//     uint8_t ret = music_db_get_level(id, difficulty);
//     if(ret > 10) {
//         music_db_entry_t *song = music_from_id(id);
//         debug_music_entry(song);
//         log_body_fatal("ultimate", "music_db_get_level(%d, %d) -> %d", id, difficulty, ret);
//     } else {
//         log_body_misc("ultimate", "music_db_get_level(%d, %d) -> %d", id, difficulty, ret);
//     }

//     return ret;
// }

// returns the fractional part of levels, ie 9.4 -> 4
uint8_t __cdecl music_db_get_level_detail(int id, uint8_t difficulty) {
    // real code has handling to ignore level < 9, but the extra function
    // music_db_is_displayable_level_detail removes the need for it

    music_db_entry_t *song = music_from_id(id);
    if(!song) {
        return 0;
    }

    float diff;
    switch(difficulty) {
        case 0:
            diff = song->detail_level_bsc;
            break;
        case 1:
            diff = song->detail_level_adv;
            break;
        case 2:
            diff = song->detail_level_ext;
            break;
        default:
            return 0;
    }

    return ((uint8_t)round(diff * 10.0)) % 10;
}

int __cdecl music_db_get_music_name_head_index(int id) {
    music_db_entry_t *song = music_from_id(id);
    // the bottom 12 bits are the sort order inside the category
    // the upper 20 bits (of which 10 are used) are the first letter of the
    // sort, with a single bit set ie (upper = 1<<letter)
    // this returns just that main sort category
    return song ? song->name_sort_id_j >> 12 : 0;
}

int __cdecl music_db_get_music_name_index(int id) {
    music_db_entry_t *song = music_from_id(id);
    return song ? song->name_sort_id_j : 0;
}

int __cdecl music_db_get_parent_music_id(int id) {
    music_db_entry_t *song = music_from_id(id);
    return song ? song->parent_id : 0;
}

uint8_t *__cdecl music_db_get_permitted_music_flag() {
    static uint8_t flags[FLAG_LEN];
    memset(flags, -1, sizeof(flags));
    return flags;
}

int16_t __cdecl music_db_get_pos_index(int id) {
    music_db_entry_t *song = music_from_id(id);
    return song ? song->pos_index : -1;
}

// bool __cdecl music_db_is_all_yellow() {
//     log_body_warning("ultimate", "music_db_is_all_yellow: not implemented");
//     return 0;
// }

// bool __cdecl music_db_is_displayable_level_detail() {
//     log_body_warning("ultimate", "music_db_is_displayable_level_detail: not implemented");
//     return 0;
// }

bool __cdecl music_db_is_exists_table(int id) {
    music_db_entry_t *song = music_from_id(id);
    return song != NULL;
}

static int version_bit_count(int id) {
    music_db_entry_t *song = music_from_id(id);
    if(!song) {
        return 0;
    }

    int bits = 0;
    while(!((1 << bits) & song->version)) {
        if(++bits >= 14) {
            break;
        }
    }

    return bits;
}

bool __cdecl music_db_is_exists_version_from_ver1(int id) {
    int bits = version_bit_count(id);
    return bits == 0 || bits == 1;
}

bool __cdecl music_db_is_exists_version_from_ver2(int id) {
    int bits = version_bit_count(id);
    return bits == 2 || bits == 3;
}

bool __cdecl music_db_is_exists_version_from_ver3(int id) {
    int bits = version_bit_count(id);
    return bits == 4 || bits == 5;
}

bool __cdecl music_db_is_exists_version_from_ver4(int id) {
    int bits = version_bit_count(id);
    return bits == 6 || bits == 7;
}

bool __cdecl music_db_is_exists_version_from_ver5(int id) {
    int bits = version_bit_count(id);
    return bits == 8 || bits == 9;
}

bool __cdecl music_db_is_exists_version_from_ver5_5(int id) {
    return version_bit_count(id) == 9;
}

bool __cdecl music_db_is_exists_version_from_ver6(int id) {
    return version_bit_count(id) == 10;
}

bool __cdecl music_db_is_exists_version_from_ver7(int id) {
    return version_bit_count(id) == 11;
}

bool __cdecl music_db_is_exists_version_from_ver8(int id) {
    return version_bit_count(id) == 12;
}

bool __cdecl music_db_is_exists_version_from_ver9(int id) {
    int bits = version_bit_count(id);
    return bits == 13 || bits == 14;
}

bool __cdecl music_db_is_hold_marker(int id) {
    music_db_entry_t *song = music_from_id(id);
    return song ? song->is_hold : 0;
}

bool __cdecl music_db_is_matched_select_type(uint8_t type, int id, uint8_t difficulty) {
    int8_t level = (int8_t)music_db_get_level(id, difficulty);

    switch(type) {
        case 1:
            return (level - 1) <= 2;
        case 2:
            return (level - 4) <= 2;
        case 3:
            return (level - 7) <= 1;
        case 4:
            return (level - 9) <= 1;
        default:
            return true;
    }
}

// bool __cdecl music_db_is_matching_select() {
//     log_body_warning("ultimate", "music_db_is_matching_select: not implemented");
//     return 0;
// }

// bool __cdecl music_db_is_nearly_excellent() {
//     log_body_warning("ultimate", "music_db_is_nearly_excellent: not implemented");
//     return 0;
// }

// bool __cdecl music_db_is_nearly_fullcombo() {
//     log_body_warning("ultimate", "music_db_is_nearly_fullcombo: not implemented");
//     return 0;
// }

// this one's a bit complex, just do the best we can with the base data
// bool __cdecl music_db_is_new() {
//     log_body_warning("ultimate", "music_db_is_new: not implemented");
//     return 0;
// }

// bool __cdecl music_db_is_no_gray() {
//     log_body_warning("ultimate", "music_db_is_no_gray: not implemented");
//     return 0;
// }

bool __cdecl music_db_is_permitted(int id) {
    return music_db_is_exists_table(id);
}

// this is used along with the hot_music_list bitfield to determine
// pickup/common in jubility rankings. You can't just return every song or the
// common folder will be empty. Every *other* place it appears should just
// return true all the time as an "unlock all" patch.
// hot_music_list THANKFULLY is one of two lists that comes in from .data, every
// other call uses a stack variable. By detecting this we can only perform
// filtering when we absolutely have to.
bool __cdecl music_db_is_possession_for_contained_music_list(uint8_t flags[FLAG_LEN], int id) {
    static uint8_t *hot_music = NULL;
    static uint8_t *data_start = NULL;
    static uint8_t *data_end = NULL;

    if(data_start == NULL) {
        auto dll_dos = (PIMAGE_DOS_HEADER) GetModuleHandle("jubeat.dll");
        GFAssert(dll_dos->e_magic == IMAGE_DOS_SIGNATURE);

        auto nt_headers = (PIMAGE_NT_HEADERS) ((uint8_t*) dll_dos + dll_dos->e_lfanew);

        // iterate sections
        auto section_count = nt_headers->FileHeader.NumberOfSections;
        PIMAGE_SECTION_HEADER section_header = IMAGE_FIRST_SECTION(nt_headers);
        for (size_t i = 0; i < section_count; section_header++, i++) {
            if(strcmp(".data", (char*)section_header->Name) != 0) {
                continue;
            }

            data_start = (uint8_t*)((DWORD)dll_dos + section_header->VirtualAddress);
            data_end = data_start + section_header->Misc.VirtualSize;

            log_body_misc("ultimate", ".data found from %p to %p", data_start, data_end);
        }

        GFAssert(data_start != NULL);
        GFAssert(data_end != NULL);
    }

    if(hot_music == NULL &&
            flags >= data_start &&
            flags < data_end &&
            // hot_music lives at a significantly larger offset than the others
            (flags - data_start) > 0x1000000) {
        hot_music = flags;
        log_body_misc("ultimate", "hot_music found at %p", hot_music);
    }

    // log_body_misc("ultimate", "%s(%p, %d)", __func__, flags, id);

    // static void* last_flags = NULL;
    // if(flags != last_flags) {
    //     last_flags = flags;
    //     for(int i = 0; i < FLAG_LEN; i+=8) {
    //         log_body_misc("ultimate", "%s %d: %X %X %X %X %X %X %X %X",
    //             __func__,
    //             i,
    //             flags[i+0],
    //             flags[i+1],
    //             flags[i+2],
    //             flags[i+3],
    //             flags[i+4],
    //             flags[i+5],
    //             flags[i+6],
    //             flags[i+7]
    //         );
    //     }
    // }

    // static uint8_t all_ff[FLAG_LEN] = {0};
    // if(all_ff[0] == 0) {
    //     memset(all_ff, 0xff, sizeof(all_ff));
    // }

    // where we just unlock things
    if(flags != hot_music) {
    //if(memcmp(all_ff, flags, FLAG_LEN) == 0) {
        return music_db_is_exists_table(id);
    }

    // jubility stuff
    music_db_entry_t *song = music_from_id(id);
    if(!song) {
        return false;
    }

    if(song->pos_index < 0 || song->pos_index >= MAX_SONGS_STOCK) {
        return 0;
    }

    size_t flag_byte = song->pos_index / 8;
    size_t flag_bit  = song->pos_index % 8;

    return (flags[flag_byte] & (1<<flag_bit)) != 0;
}

// ID test
// bool __cdecl music_db_is_random_or_matching_select() {
//     log_body_warning("ultimate", "music_db_is_random_or_matching_select: not implemented");
//     return 0;
// }

// bool __cdecl music_db_is_random_select() {
//     log_body_warning("ultimate", "music_db_is_random_select: not implemented");
//     return 0;
// }

// int __cdecl music_db_music_bar_to_dot_array() {
//     log_body_warning("ultimate", "music_db_music_bar_to_dot_array: not implemented");
//     return 0;
// }

// int __cdecl music_db_set_default_add_music_flag(uint8_t flags[FLAG_LEN]) {
//     return 0;
// }

// int __cdecl music_db_set_flag_equivalent_for_music_id(void *a1, unsigned int flag, int value) {
//     // I *think* this can do nothing
//     //log_body_warning("ultimate", "music_db_set_flag_equivalent_for_music_id(%p, %d, %d): not implemented", a1, flag, value);
//     return 0;
// }

// int __cdecl music_db_set_permitted_music_flag(uint8_t flags[FLAG_LEN]) {
//     return 0;
// }

// these functions handily replace all the music_record functions because
// they call GFHashMapKeyToValue before checking size limits
static int music_record_count;
static uint8_t music_records[MAX_SONGS][2112]; // dll says 2112 bytes each
static robin_hood::unordered_map<int, void*> music_record_map;

void* GFHashMapCreate(void *mem, int mem_sz, int max_elems) {
    log_body_info("ultimate", "Hooked GFHashMapCreate");
    if(music_count == 0) {
        log_body_fatal("ultimate", "GFHashMapCreate called before mdb load, cannot continue");
    }

    memset(music_records, 0, sizeof(music_records));
    music_record_count = 0;
    music_record_map.clear();

    // pre-populate the hashmap to avoid issues down the track
    for(int i = 0; i < music_count; i++) {
        music_record_map[music_db[i].music_id] = music_records[music_record_count++];
    }

    return &music_record_map;
}

void GFHashMapRegist(void *map, int key, void *val) {
    log_body_fatal("ultimate", "GFHashMapRegist should not be called if patches worked");
}

void* GFHashMapKeyToValue(void *map, int key) {
    if(music_count == 0) {
        log_body_fatal("ultimate", "GFHashMapKeyToValue called before mdb load, cannot continue");
    }

    auto search = music_record_map.find(key);
    if(search == music_record_map.end()) {
        if(music_record_count >= MAX_SONGS) {
            log_body_fatal("ultimate", "music records full even with gratuitous buffer");
        }

        // insert new, wasn't in the music db but maybe tutorial or something
        //log_body_warning("ultimate", "music_record added unknown ID %d", key);
        music_record_map[key] = music_records[music_record_count];
        return music_records[music_record_count++];
    } else {
        return search->second;
    };
}

// this has potential for failure, but it works for now
static int music_record_iter;

void GFHashMapRewindEntryList(void *map) {
    music_record_iter = 0;
}

bool GFHashMapGetEntryList(void *map, int *key, void **val) {
    if(music_record_iter >= music_count) {
        return false;
    }

    *key = music_db[music_record_iter].music_id;
    *val = music_records[music_record_iter];
    music_record_iter++;

    return true;
}

// internal 100 element list?
// int __cdecl music_db_set_select_history_list() {
//     log_body_warning("ultimate", "music_db_set_select_history_list: not implemented");
//     return 0;
// }


// int __cdecl music_bonus_get_bonus_music() {
//     log_body_warning("ultimate", "music_bonus_get_bonus_music: not implemented");
//     return 0;
// }
// int __cdecl music_bonus_is_bonus_music() {
//     log_body_warning("ultimate", "music_bonus_is_bonus_music: not implemented");
//     return 0;
// }
// only_now functions: some internal list, only 12 elements long. Passthrough.
// int __cdecl music_only_now_get_count() {
//     log_body_warning("ultimate", "music_only_now_get_count: not implemented");
//     return 0;
// }
// int __cdecl music_only_now_get_etime() {
//     log_body_warning("ultimate", "music_only_now_get_etime: not implemented");
//     return 0;
// }
// int __cdecl music_only_now_get_music_id() {
//     log_body_warning("ultimate", "music_only_now_get_music_id: not implemented");
//     return 0;
// }
// bool __cdecl music_only_now_initialize() {
//     return true;
// }
// int __cdecl music_record_add_clear_count() {
//     log_body_warning("ultimate", "music_record_add_clear_count: not implemented");
//     return 0;
// }
// int __cdecl music_record_add_excellent_count() {
//     log_body_warning("ultimate", "music_record_add_excellent_count: not implemented");
//     return 0;
// }
// int __cdecl music_record_add_full_combo_count() {
//     log_body_warning("ultimate", "music_record_add_full_combo_count: not implemented");
//     return 0;
// }
// int __cdecl music_record_add_play_count() {
//     log_body_warning("ultimate", "music_record_add_play_count: not implemented");
//     return 0;
// }
// int __cdecl music_record_clear_context() {
//     log_body_warning("ultimate", "music_record_clear_context: not implemented");
//     return 0;
// }
// int __cdecl music_record_get_best_music_rate() {
//     log_body_warning("ultimate", "music_record_get_best_music_rate: not implemented");
//     return 0;
// }
// int __cdecl music_record_get_best_score() {
//     log_body_warning("ultimate", "music_record_get_best_score: not implemented");
//     return 0;
// }
// int __cdecl music_record_get_clear_count() {
//     log_body_warning("ultimate", "music_record_get_clear_count: not implemented");
//     return 0;
// }
// int __cdecl music_record_get_clear_flag() {
//     log_body_warning("ultimate", "music_record_get_clear_flag: not implemented");
//     return 0;
// }
// int __cdecl music_record_get_excellent_count() {
//     log_body_warning("ultimate", "music_record_get_excellent_count: not implemented");
//     return 0;
// }
// int __cdecl music_record_get_full_combo_count() {
//     log_body_warning("ultimate", "music_record_get_full_combo_count: not implemented");
//     return 0;
// }
// int __cdecl music_record_get_jubility() {
//     log_body_warning("ultimate", "music_record_get_jubility: not implemented");
//     return 0;
// }
// int __cdecl music_record_get_music_bar() {
//     log_body_warning("ultimate", "music_record_get_music_bar: not implemented");
//     return 0;
// }
// int __cdecl music_record_get_play_count() {
//     log_body_warning("ultimate", "music_record_get_play_count: not implemented");
//     return 0;
// }
// int __cdecl music_record_get_sequence_record_set() {
//     log_body_warning("ultimate", "music_record_get_sequence_record_set: not implemented");
//     return 0;
// }
// int __cdecl music_record_get_total_best_score() {
//     log_body_warning("ultimate", "music_record_get_total_best_score: not implemented");
//     return 0;
// }
// int __cdecl music_record_is_all_played() {
//     log_body_warning("ultimate", "music_record_is_all_played: not implemented");
//     return 0;
// }
// int __cdecl music_record_is_all_yellow() {
//     log_body_warning("ultimate", "music_record_is_all_yellow: not implemented");
//     return 0;
// }
// int __cdecl music_record_is_any_played() {
//     log_body_warning("ultimate", "music_record_is_any_played: not implemented");
//     return 0;
// }
// int __cdecl music_record_is_cleared() {
//     log_body_warning("ultimate", "music_record_is_cleared: not implemented");
//     return 0;
// }
// int __cdecl music_record_is_excellent() {
//     log_body_warning("ultimate", "music_record_is_excellent: not implemented");
//     return 0;
// }
// int __cdecl music_record_is_full_combo() {
//     log_body_warning("ultimate", "music_record_is_full_combo: not implemented");
//     return 0;
// }
// int __cdecl music_record_is_no_gray() {
//     log_body_warning("ultimate", "music_record_is_no_gray: not implemented");
//     return 0;
// }
// int __cdecl music_record_is_played(int id, uint8_t a2, int a3) {
//     log_body_warning("ultimate", "music_record_is_played(%d, %d, %d): not implemented",id,a2,a3);
//     return 0;
// }
// int __cdecl music_record_merge_music_bar() {
//     log_body_warning("ultimate", "music_record_merge_music_bar: not implemented");
//     return 0;
// }
// int __cdecl music_record_set_best_music_rate() {
//     log_body_warning("ultimate", "music_record_set_best_music_rate: not implemented");
//     return 0;
// }
// int __cdecl music_record_set_best_score() {
//     log_body_warning("ultimate", "music_record_set_best_score: not implemented");
//     return 0;
// }
// int __cdecl music_record_set_clear_count() {
//     log_body_warning("ultimate", "music_record_set_clear_count: not implemented");
//     return 0;
// }
// int __cdecl music_record_set_clear_flag() {
//     log_body_warning("ultimate", "music_record_set_clear_flag: not implemented");
//     return 0;
// }
// int __cdecl music_record_set_excellent_count() {
//     log_body_warning("ultimate", "music_record_set_excellent_count: not implemented");
//     return 0;
// }
// int __cdecl music_record_set_full_combo_count() {
//     log_body_warning("ultimate", "music_record_set_full_combo_count: not implemented");
//     return 0;
// }
// int __cdecl music_record_set_jubility() {
//     log_body_warning("ultimate", "music_record_set_jubility: not implemented");
//     return 0;
// }
// int __cdecl music_record_set_play_count() {
//     log_body_warning("ultimate", "music_record_set_play_count: not implemented");
//     return 0;
// }
// shareable - tentative "not needed"
// int __cdecl music_shareable_add_shareable_music() {
//     log_body_warning("ultimate", "music_shareable_add_shareable_music: not implemented");
//     return 0;
// }
// bool __cdecl music_shareable_initialize() {
//     return 1;
// }
// int __cdecl music_shareable_is_shareable_music() {
//     log_body_warning("ultimate", "music_shareable_is_shareable_music: not implemented");
//     return 1;
// }
// void __cdecl music_shareable_set_flag(uint8_t flags[FLAG_LEN]) {
// }

// void *__cdecl music_texture_BlackJacket_GetInstance() {
//     return NULL;
// }
// int __cdecl music_texture_BlackJacket_ReadXmlNode() {
//     return 0;
// }
// int __cdecl music_bonus_weekly_clear() {
//     log_body_warning("ultimate", "music_bonus_weekly_clear: not implemented");
//     return 0;
// }
// int __cdecl music_bonus_weekly_music_bonus_get_target_music_info() {
//     log_body_warning("ultimate", "music_bonus_weekly_music_bonus_get_target_music_info: not implemented");
//     return 0;
// }
// int __cdecl music_bonus_weekly_music_bonus_weekly_is_target_music() {
//     log_body_warning("ultimate", "music_bonus_weekly_music_bonus_weekly_is_target_music: not implemented");
//     return 0;
// }
// int __cdecl music_new_clear() {
//     log_body_warning("ultimate", "music_new_clear: not implemented");
//     return 0;
// }
// int __cdecl music_new_get_list() {
//     log_body_warning("ultimate", "music_new_get_list: not implemented");
//     return 0;
// }
// int __cdecl music_new_read_xml_node() {
//     log_body_warning("ultimate", "music_new_read_xml_node: not implemented");
//     return 0;
// }

// intentionally ignored:
// music_db_is_tutorial
