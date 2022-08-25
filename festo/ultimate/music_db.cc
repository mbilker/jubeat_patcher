#define LOG_MODULE "ultimate::music_db"

#include <cstdint>
#include <cstdlib>

#include <math.h>

// clang-format off
#include <windows.h>
// clang-format on

#include "imports/avs2-core/avs.h"
#include "imports/gftools.h"

#include "pe/iat.h"

#include "util/lib.h"
#include "util/log.h"
#include "util/robin_hood.h"
#include "util/str.h"

#include "music_db.h"

// There is a lot in flux here so let's make this easier
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif

// default: 2 MB
// ultimate: 6 MB
#define MDB_XML_SIZE (6 * 1024 * 1024)

// include this #define to validate every song in the MDB has an associated
// ifs file in the ifs_pack folder
#define DEBUG_CHECK_MUSIC_IFS_EXISTS

struct avs_stat {
    uint64_t st_atime;
    uint64_t st_mtime;
    uint64_t st_ctime;
    int32_t unk1;
    uint32_t filesize;
    // not actually sure how big theirs is
    struct stat padding;
};

#ifdef DEBUG_CHECK_MUSIC_IFS_EXISTS
int (*avs_fs_lstat)(const char *path, struct avs_stat *st);
#endif

static bool __cdecl music_db_get_sequence_filename(void *a1, void *a2, int music_id, uint8_t seq);
static bool __cdecl music_db_get_sound_filename(void *a1, void *a2, int music_id, uint8_t seq);

// bool __cdecl music_db_finalize(void);
static bool __cdecl music_db_initialize(void);
static decltype(music_db_initialize) *music_db_initialize_orig = nullptr;
// bool __cdecl music_db_reset_using_datapackage(int a1);
// int __cdecl music_db_dbg_get_all_list(void);
// int __cdecl music_db_dot_array_to_music_bar(void);
static int __cdecl music_db_get_default_list(int limit, int *results);
static int __cdecl music_db_get_offline_default_list(int limit, int *results);
static int __cdecl music_db_get_all_permitted_list(int limit, int *results);
static int __cdecl music_db_get_possession_list(uint8_t flags[FLAG_LEN], int limit, int *results);
static int __cdecl music_db_get_card_default_list(int limit, int *results);
// int __cdecl music_db_get_jukebox_list();
static float __cdecl music_db_get_bpm(int id);
static float __cdecl music_db_get_bpm_min(int id);
// int __cdecl music_db_get_default_id();
// int __cdecl music_db_get_default_id_by_genre();
// int __cdecl music_db_get_default_id_by_mode();
static char *__cdecl music_db_get_genre_list(int id);
static uint64_t __cdecl music_db_get_grouping_category_list(int id);
static int __cdecl music_db_get_index_start(int id);
static uint8_t __cdecl music_db_get_level(int id, uint8_t difficulty);
// hook this instead if you're having issues
// uint8_t __cdecl music_db_get_level_logged(int id, uint8_t difficulty);
static uint8_t __cdecl music_db_get_level_detail(int id, uint8_t difficulty);
static int __cdecl music_db_get_music_name_head_index(int id);
static int __cdecl music_db_get_music_name_index(int id);
static int __cdecl music_db_get_parent_music_id(int id);
static uint8_t *__cdecl music_db_get_permitted_music_flag();
static int16_t __cdecl music_db_get_pos_index(int a1);
// bool __cdecl music_db_is_all_yellow();
// bool __cdecl music_db_is_displayable_level_detail(void);
static bool __cdecl music_db_is_exists_table(int id);
static bool __cdecl music_db_is_exists_version_from_ver1(int id);
static bool __cdecl music_db_is_exists_version_from_ver2(int id);
static bool __cdecl music_db_is_exists_version_from_ver3(int id);
static bool __cdecl music_db_is_exists_version_from_ver4(int id);
static bool __cdecl music_db_is_exists_version_from_ver5(int id);
static bool __cdecl music_db_is_exists_version_from_ver5_5(int id);
static bool __cdecl music_db_is_exists_version_from_ver6(int id);
static bool __cdecl music_db_is_exists_version_from_ver7(int id);
static bool __cdecl music_db_is_exists_version_from_ver8(int id);
static bool __cdecl music_db_is_exists_version_from_ver9(int id);
static bool __cdecl music_db_is_hold_marker(int id);
static bool __cdecl music_db_is_matched_select_type(uint8_t type, int id, uint8_t difficulty);
// bool __cdecl music_db_is_matching_select();
// bool __cdecl music_db_is_nearly_excellent();
// bool __cdecl music_db_is_nearly_fullcombo();
// bool __cdecl music_db_is_new();
// bool __cdecl music_db_is_no_gray();
static bool __cdecl music_db_is_permitted(int id);
static bool __cdecl music_db_is_possession_for_contained_music_list(
    uint8_t flags[FLAG_LEN], int a2);
// bool __cdecl music_db_is_random_or_matching_select();
// bool __cdecl music_db_is_random_select();
// int __cdecl music_db_music_bar_to_dot_array();
// int __cdecl music_db_set_default_add_music_flag(uint8_t flags[FLAG_LEN]);
// int __cdecl music_db_set_flag_equivalent_for_music_id(void *a1, unsigned int flag, int value);
// int __cdecl music_db_set_permitted_music_flag(uint8_t flags[FLAG_LEN]);
// int __cdecl music_db_set_select_history_list();

// int __cdecl music_bonus_get_bonus_music();
// int __cdecl music_bonus_is_bonus_music();
// int __cdecl music_only_now_get_count();
// int __cdecl music_only_now_get_etime();
// int __cdecl music_only_now_get_music_id();
// bool __cdecl music_only_now_initialize();
// int __cdecl music_record_add_clear_count();
// int __cdecl music_record_add_excellent_count();
// int __cdecl music_record_add_full_combo_count();
// int __cdecl music_record_add_play_count();
// int __cdecl music_record_clear_context();
// int __cdecl music_record_get_best_music_rate();
// int __cdecl music_record_get_best_score();
// int __cdecl music_record_get_clear_count();
// int __cdecl music_record_get_clear_flag();
// int __cdecl music_record_get_excellent_count();
// int __cdecl music_record_get_full_combo_count();
// int __cdecl music_record_get_jubility();
// int __cdecl music_record_get_music_bar();
// int __cdecl music_record_get_play_count();
// int __cdecl music_record_get_sequence_record_set();
// int __cdecl music_record_get_total_best_score();
// int __cdecl music_record_is_all_played();
// int __cdecl music_record_is_all_yellow();
// int __cdecl music_record_is_any_played();
// int __cdecl music_record_is_cleared();
// int __cdecl music_record_is_excellent();
// int __cdecl music_record_is_full_combo();
// int __cdecl music_record_is_no_gray();
// int __cdecl music_record_is_played(int id, uint8_t a2, int a3);
// int __cdecl music_record_merge_music_bar();
// int __cdecl music_record_set_best_music_rate();
// int __cdecl music_record_set_best_score();
// int __cdecl music_record_set_clear_count();
// int __cdecl music_record_set_clear_flag();
// int __cdecl music_record_set_excellent_count();
// int __cdecl music_record_set_full_combo_count();
// int __cdecl music_record_set_jubility();
// int __cdecl music_record_set_play_count();
// int __cdecl music_shareable_add_shareable_music();
// bool __cdecl music_shareable_initialize();
// int __cdecl music_shareable_is_shareable_music();
// void __cdecl music_shareable_set_flag(uint8_t flags[FLAG_LEN]);

// void *__cdecl music_texture_BlackJacket_GetInstance();
// int __cdecl music_texture_BlackJacket_ReadXmlNode();
// int __cdecl music_bonus_weekly_music_bonus_get_target_music_info();
// int __cdecl music_bonus_weekly_clear();
// int __cdecl music_bonus_weekly_music_bonus_weekly_is_target_music();
// int __cdecl music_new_clear();
// int __cdecl music_new_get_list();
// int __cdecl music_new_read_xml_node();

static void __cdecl GFHashMapRegist(void *map, int key, void *val);
static void *__cdecl GFHashMapCreate(void *mem, int mem_sz, int max_elems);
static void *__cdecl GFHashMapKeyToValue(void *map, int key);
static bool __cdecl GFHashMapGetEntryList(void *map, int *key, void **val);
static void __cdecl GFHashMapRewindEntryList(void *map);

static const struct hook_symbol music_db_hooks[] = {
    {
        .name = "music_db_get_sequence_filename",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_db_get_sequence_filename),
        .link = nullptr,
    },
    {
        .name = "music_db_get_sound_filename",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_db_get_sound_filename),
        .link = nullptr,
    },
    /*
    {
        .name = "music_db_dbg_get_all_list",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_db_dbg_get_all_list),
        .link = nullptr,
    },
    {
        .name = "music_db_dot_array_to_music_bar",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_db_dot_array_to_music_bar),
        .link = nullptr,
    },
    {
        .name = "music_db_finalize",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_db_finalize),
        .link = nullptr,
    },
    */
    {
        .name = "music_db_get_bpm",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_db_get_bpm),
        .link = nullptr,
    },
    {
        .name = "music_db_get_bpm_min",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_db_get_bpm_min),
        .link = nullptr,
    },
    {
        .name = "music_db_get_default_list",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_db_get_default_list),
        .link = nullptr,
    },
    {
        .name = "music_db_get_offline_default_list",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_db_get_offline_default_list),
        .link = nullptr,
    },
    {
        .name = "music_db_get_all_permitted_list",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_db_get_all_permitted_list),
        .link = nullptr,
    },
    {
        .name = "music_db_get_possession_list",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_db_get_possession_list),
        .link = nullptr,
    },
    {
        .name = "music_db_get_card_default_list",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_db_get_card_default_list),
        .link = nullptr,
    },
    /*
    {
        .name = "music_db_get_jukebox_list",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_db_get_jukebox_list),
        .link = nullptr,
    },
    {
        .name = "music_db_get_default_id",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_db_get_default_id),
        .link = nullptr,
    },
    {
        .name = "music_db_get_default_id_by_genre",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_db_get_default_id_by_genre),
        .link = nullptr,
    },
    {
        .name = "music_db_get_default_id_by_mode",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_db_get_default_id_by_mode),
        .link = nullptr,
    },
    */
    {
        .name = "music_db_get_genre_list",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_db_get_genre_list),
        .link = nullptr,
    },
    {
        .name = "music_db_get_grouping_category_list",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_db_get_grouping_category_list),
        .link = nullptr,
    },
    {
        .name = "music_db_get_index_start",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_db_get_index_start),
        .link = nullptr,
    },
    {
        .name = "music_db_get_level",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_db_get_level),
        .link = nullptr,
    },
    {
        .name = "music_db_get_level_detail",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_db_get_level_detail),
        .link = nullptr,
    },
    {
        .name = "music_db_get_music_name_head_index",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_db_get_music_name_head_index),
        .link = nullptr,
    },
    {
        .name = "music_db_get_music_name_index",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_db_get_music_name_index),
        .link = nullptr,
    },
    {
        .name = "music_db_get_parent_music_id",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_db_get_parent_music_id),
        .link = nullptr,
    },
    {
        .name = "music_db_get_permitted_music_flag",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_db_get_permitted_music_flag),
        .link = nullptr,
    },
    {
        .name = "music_db_get_pos_index",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_db_get_pos_index),
        .link = nullptr,
    },
    {
        .name = "music_db_initialize",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_db_initialize),
        .link = reinterpret_cast<void **>(&music_db_initialize_orig),
    },
    /*
    {
        .name = "music_db_is_all_yellow",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_db_is_all_yellow),
        .link = nullptr,
    },
    {
        .name = "music_db_is_displayable_level_detail",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_db_is_displayable_level_detail),
        .link = nullptr,
    },
    */
    {
        .name = "music_db_is_exists_table",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_db_is_exists_table),
        .link = nullptr,
    },
    {
        .name = "music_db_is_exists_version_from_ver1",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_db_is_exists_version_from_ver1),
        .link = nullptr,
    },
    {
        .name = "music_db_is_exists_version_from_ver2",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_db_is_exists_version_from_ver2),
        .link = nullptr,
    },
    {
        .name = "music_db_is_exists_version_from_ver3",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_db_is_exists_version_from_ver3),
        .link = nullptr,
    },
    {
        .name = "music_db_is_exists_version_from_ver4",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_db_is_exists_version_from_ver4),
        .link = nullptr,
    },
    {
        .name = "music_db_is_exists_version_from_ver5",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_db_is_exists_version_from_ver5),
        .link = nullptr,
    },
    {
        .name = "music_db_is_exists_version_from_ver5_5",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_db_is_exists_version_from_ver5_5),
        .link = nullptr,
    },
    {
        .name = "music_db_is_exists_version_from_ver6",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_db_is_exists_version_from_ver6),
        .link = nullptr,
    },
    {
        .name = "music_db_is_exists_version_from_ver7",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_db_is_exists_version_from_ver7),
        .link = nullptr,
    },
    {
        .name = "music_db_is_exists_version_from_ver8",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_db_is_exists_version_from_ver8),
        .link = nullptr,
    },
    {
        .name = "music_db_is_exists_version_from_ver9",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_db_is_exists_version_from_ver9),
        .link = nullptr,
    },
    {
        .name = "music_db_is_hold_marker",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_db_is_hold_marker),
        .link = nullptr,
    },
    {
        .name = "music_db_is_matched_select_type",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_db_is_matched_select_type),
        .link = nullptr,
    },
    /*
    {
        .name = "music_db_is_matching_select",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_db_is_matching_select),
        .link = nullptr,
    },
    {
        .name = "music_db_is_nearly_excellent",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_db_is_nearly_excellent),
        .link = nullptr,
    },
    {
        .name = "music_db_is_nearly_fullcombo",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_db_is_nearly_fullcombo),
        .link = nullptr,
    },
    {
        .name = "music_db_is_new",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_db_is_new),
        .link = nullptr,
    },
    {
        .name = "music_db_is_no_gray",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_db_is_no_gray),
        .link = nullptr,
    },
    */
    {
        .name = "music_db_is_permitted",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_db_is_permitted),
        .link = nullptr,
    },
    {
        .name = "music_db_is_possession_for_contained_music_list",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_db_is_possession_for_contained_music_list),
        .link = nullptr,
    },
    /*
    {
        .name = "music_db_is_random_or_matching_select",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_db_is_random_or_matching_select),
        .link = nullptr,
    },
    {
        .name = "music_db_is_random_select",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_db_is_random_select),
        .link = nullptr,
    },
    {
        .name = "music_db_music_bar_to_dot_array",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_db_music_bar_to_dot_array),
        .link = nullptr,
    },
    {
        .name = "music_db_reset_using_datapackage",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_db_reset_using_datapackage),
        .link = nullptr,
    },
    {
        .name = "music_db_set_default_add_music_flag",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_db_set_default_add_music_flag),
        .link = nullptr,
    },
    {
        .name = "music_db_set_flag_equivalent_for_music_id",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_db_set_flag_equivalent_for_music_id),
        .link = nullptr,
    },
    {
        .name = "music_db_set_permitted_music_flag",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_db_set_permitted_music_flag),
        .link = nullptr,
    },
    {
        .name = "music_db_set_select_history_list",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_db_set_select_history_list),
        .link = nullptr,
    },
    */
    /*
    {
        .name = "music_bonus_get_bonus_music",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_bonus_get_bonus_music),
        .link = nullptr,
    },
    {
        .name = "music_bonus_is_bonus_music",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_bonus_is_bonus_music),
        .link = nullptr,
    },
    {
        .name = "music_only_now_get_count",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_only_now_get_count),
        .link = nullptr,
    },
    {
        .name = "music_only_now_get_etime",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_only_now_get_etime),
        .link = nullptr,
    },
    {
        .name = "music_only_now_get_music_id",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_only_now_get_music_id),
        .link = nullptr,
    {
    },
        .name = "music_only_now_initialize",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_only_now_initialize),
        .link = nullptr,
    },
    */
    // TODO: all music_record function hooks are broken
    /*
    {
        .name = "music_record_add_clear_count",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_record_add_clear_count),
        .link = nullptr,
    },
    {
        .name = "music_record_add_excellent_count",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_record_add_excellent_count),
        .link = nullptr,
    },
    {
        .name = "music_record_add_full_combo_count",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_record_add_full_combo_count),
        .link = nullptr,
    },
    {
        .name = "music_record_add_play_count",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_record_add_play_count),
        .link = nullptr,
    },
    {
        .name = "music_record_clear_context",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_record_clear_context),
        .link = nullptr,
    },
    {
        .name = "music_record_get_best_music_rate",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_record_get_best_music_rate),
        .link = nullptr,
    },
    {
        .name = "music_record_get_best_score",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_record_get_best_score),
        .link = nullptr,
    },
    {
        .name = "music_record_get_clear_count",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_record_get_clear_count),
        .link = nullptr,
    },
    {
        .name = "music_record_get_clear_flag",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_record_get_clear_flag),
        .link = nullptr,
    },
    {
        .name = "music_record_get_excellent_count",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_record_get_excellent_count),
        .link = nullptr,
    },
    {
        .name = "music_record_get_full_combo_count",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_record_get_full_combo_count),
        .link = nullptr,
    },
    {
        .name = "music_record_get_jubility",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_record_get_jubility),
        .link = nullptr,
    },
    {
        .name = "music_record_get_music_bar",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_record_get_music_bar),
        .link = nullptr,
    },
    {
        .name = "music_record_get_play_count",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_record_get_play_count),
        .link = nullptr,
    },
    {
        .name = "music_record_get_sequence_record_set",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_record_get_sequence_record_set),
        .link = nullptr,
    },
    {
        .name = "music_record_get_total_best_score",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_record_get_total_best_score),
        .link = nullptr,
    },
    {
        .name = "music_record_is_all_played",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_record_is_all_played),
        .link = nullptr,
    },
    {
        .name = "music_record_is_all_yellow",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_record_is_all_yellow),
        .link = nullptr,
    },
    {
        .name = "music_record_is_any_played",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_record_is_any_played),
        .link = nullptr,
    },
    {
        .name = "music_record_is_cleared",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_record_is_cleared),
        .link = nullptr,
    },
    {
        .name = "music_record_is_excellent",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_record_is_excellent),
        .link = nullptr,
    },
    {
        .name = "music_record_is_full_combo",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_record_is_full_combo),
        .link = nullptr,
    },
    {
        .name = "music_record_is_no_gray",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_record_is_no_gray),
        .link = nullptr,
    },
    {
        .name = "?music_record_is_played@@YA_NIEW4MUSIC_RECORD_TYPE@@@Z",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_record_is_played),
        .link = nullptr,
    },
    {
        .name = "music_record_merge_music_bar",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_record_merge_music_bar),
        .link = nullptr,
    },
    {
        .name = "music_record_set_best_music_rate",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_record_set_best_music_rate),
        .link = nullptr,
    },
    {
        .name = "music_record_set_best_score",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_record_set_best_score),
        .link = nullptr,
    },
    {
        .name = "music_record_set_clear_count",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_record_set_clear_count),
        .link = nullptr,
    },
    {
        .name = "music_record_set_clear_flag",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_record_set_clear_flag),
        .link = nullptr,
    },
    {
        .name = "music_record_set_excellent_count",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_record_set_excellent_count),
        .link = nullptr,
    },
    {
        .name = "music_record_set_full_combo_count",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_record_set_full_combo_count),
        .link = nullptr,
    },
    {
        .name = "music_record_set_jubility",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_record_set_jubility),
        .link = nullptr,
    },
    {
        .name = "music_record_set_play_count",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_record_set_play_count),
        .link = nullptr,
    },
    {
        .name = "music_shareable_add_shareable_music",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_shareable_add_shareable_music),
        .link = nullptr,
    },
    {
        .name = "music_shareable_initialize",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_shareable_initialize),
        .link = nullptr,
    },
    {
        .name = "music_shareable_is_shareable_music",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_shareable_is_shareable_music),
        .link = nullptr,
    },
    {
        .name = "music_shareable_set_flag",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_shareable_set_flag),
        .link = nullptr,
    },
    */
    /*
    {
        .name = "?GetInstance@BlackJacket@music_texture@@SAAAV12@XZ",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_texture_BlackJacket_GetInstance),
        .link = nullptr,
    },
    {
        .name = "?ReadXmlNode@BlackJacket@music_texture@@QAE_NPAUT_PROPERTY_NODE@@@Z",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_texture_BlackJacket_ReadXmlNode),
        .link = nullptr,
    },
    {
        .name = "?music_bonus_get_target_music_info@music_bonus_weekly@@YAXPBUJBMusicFlag_T@@AA_NAAI@Z",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_bonus_weekly_music_bonus_get_target_music_info),
        .link = nullptr,
    },
    {
        .name = "?clear@music_bonus_weekly@@YAXXZ",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_bonus_weekly_clear),
        .link = nullptr,
    },
    {
        .name = "?music_bonus_weekly_is_target_music@music_bonus_weekly@@YA_NPBUJBMusicFlag_T@@I@Z",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_bonus_weekly_music_bonus_weekly_is_target_music),
        .link = nullptr,
    },
    {
        .name = "?clear@music_new@@YAXXZ",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_new_clear),
        .link = nullptr,
    },
    {
        .name = "?get_list@music_new@@YAHHQAI@Z",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_new_get_list),
        .link = nullptr,
    },
    {
        .name = "?read_xml_node@music_new@@YA_NPAUT_PROPERTY_NODE@@@Z",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(music_new_read_xml_node),
        .link = nullptr,
    },
    */
};
static const struct hook_symbol gftools_hooks[] = {
    {
        .name = "GFHashMapRegist",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(GFHashMapRegist),
        .link = nullptr,
    },
    {
        .name = "GFHashMapCreate",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(GFHashMapCreate),
        .link = nullptr,
    },
    {
        .name = "GFHashMapKeyToValue",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(GFHashMapKeyToValue),
        .link = nullptr,
    },
    {
        .name = "GFHashMapGetEntryList",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(GFHashMapGetEntryList),
        .link = nullptr,
    },
    {
        .name = "GFHashMapRewindEntryList",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(GFHashMapRewindEntryList),
        .link = nullptr,
    },
};

void hook_music_db(HANDLE process, HMODULE jubeat_handle, HMODULE music_db_handle)
{
    iat_hook_table_apply(
        process, jubeat_handle, "music_db.dll", music_db_hooks, std::size(music_db_hooks));
    iat_hook_table_apply(
        process, music_db_handle, "gftools.dll", gftools_hooks, std::size(gftools_hooks));
}

static bool __cdecl music_db_get_sequence_filename(void *a1, void *a2, int music_id, uint8_t seq)
{
    // log_misc("music_db_get_sequence_filename(%p, %p, %d, %d)", a1, a2, music_id, seq);

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

static bool __cdecl music_db_get_sound_filename(void *a1, void *a2, int music_id, uint8_t seq)
{
    // log_misc("music_db_get_sound_filename(%p, %p, %d, %d)", a1, a2, music_id,
    // seq);

    bool use_idx = music_id != 90010017 && seq > 0;

    return GFSLPrintf(a1, a2, "%s/%09d/%s.bin", "data/music", music_id, use_idx ? "idx" : "bgm") >=
           0;
}

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
    int32_t pack_id; // custom! Extend pack ID from Jubeat mobile versions
    // custom! real, human-readable name. Max seen was 64 bytes, 256 for massive
    // overkill in case of weird songs. Used to auto-generate sort IDs
    char title_name[256];
};

static int music_count;
static music_db_entry_t music_db[MAX_SONGS];
static robin_hood::unordered_map<int, music_db_entry_t *> music_db_map;

static void debug_music_entry(music_db_entry_t *song)
{
    if (!song) {
        log_body_warning("ultimate", "%s: song == nullptr", __func__);
        return;
    }
    log_body_misc("ultimate", "music_id = %d", song->music_id);
    log_body_misc("ultimate", "parent_id = %d", song->parent_id);
    log_body_misc("ultimate", "name_sort_id_j = %x", song->name_sort_id_j);
    log_body_misc("ultimate", "detail_level_bsc = %f", song->detail_level_bsc);
    log_body_misc("ultimate", "detail_level_adv = %f", song->detail_level_adv);
    log_body_misc("ultimate", "detail_level_ext = %f", song->detail_level_ext);
    log_body_misc("ultimate", "bpm_max = %f", song->bpm_max);
    log_body_misc("ultimate", "bpm_min = %f", song->bpm_min);
    log_body_misc("ultimate", "music_type = %d", song->music_type);
    log_body_misc("ultimate", "version = %x", song->version);
    log_body_misc("ultimate", "pos_index = %d", static_cast<int>(song->pos_index));
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
    log_body_misc(
        "ultimate", "grouping_category = %lX", static_cast<long int>(song->grouping_category));
    log_body_misc("ultimate", "pack_id = %d", song->pack_id);
}

enum music_load_res {
    MUSIC_LOAD_OK = 1,
    MUSIC_LOAD_BAD_VER = 2,
    MUSIC_LOAD_FULL = 3,
};

static music_db_entry_t *music_from_id(int id)
{
    auto search = music_db_map.find(id);

    return search == music_db_map.end() ? nullptr : search->second;
}

typedef bool (*music_filter_func)(music_db_entry_t *song);

static bool filter_func_all(music_db_entry_t *song)
{
    return true;
}

// the non-extend stuff falls below the 2048 song limit (about 1300 songs)
// this is thus a nice easy way to get the arcade tracks into jubility calcs
static bool filter_func_not_extend(music_db_entry_t *song)
{
    return song->pack_id == -1;
}

static bool filter_func_is_default(music_db_entry_t *song)
{
    return song->is_default;
}

static bool filter_func_card_default(music_db_entry_t *song)
{
    return song->is_card_default;
}

static bool filter_func_is_offline_default(music_db_entry_t *song)
{
    return song->is_offline_default;
}

static int
music_db_filtered_list(const char *func, int limit, int *results, music_filter_func filter)
{
    int returned = 0;
    int found = 0;

    for (int i = 0; i < music_count; i++) {
        music_db_entry_t *song = &music_db[i];

        if (!filter(song)) {
            continue;
        }

        found++;
        if (returned < limit) {
            returned++;
            *results++ = song->music_id;
        }
    }

    if (found != returned) {
        log_body_warning(
            "ultimate", "%s could have returned %d but capped at %d", func, found, returned);
    }

    log_body_misc("ultimate", "%s(%d, %p) -> %d", func, limit, results, returned);

    return returned;
}

static int __cdecl music_db_get_default_list(int limit, int *results)
{
    return music_db_filtered_list(__func__, limit, results, filter_func_not_extend);
    // return music_db_filtered_list(__func__, limit, results,
    // filter_func_is_default);
}

static int __cdecl music_db_get_offline_default_list(int limit, int *results)
{
    return music_db_filtered_list(__func__, limit, results, filter_func_not_extend);
    // return music_db_filtered_list(__func__, limit, results,
    // filter_func_is_offline_default);
}

// any song missing from here won't be included in jubility pick-up, and any
// song missing from here that blindly returns true in music_db_is_permitted
// will errorneously increment jubility locally when played, but reset next
// login. Patch the initial jubility function's arrays to allow full
// functionality
static int __cdecl music_db_get_all_permitted_list(int limit, int *results)
{
    return music_db_filtered_list(__func__, limit, results, filter_func_all);
}

static int __cdecl music_db_get_possession_list(uint8_t flags[FLAG_LEN], int limit, int *results)
{
    // log_body_warning("ultimate", "music_db_get_possession_list(%p, %d, %p)",
    // flags, limit, results);

    return music_db_filtered_list(__func__, limit, results, filter_func_all);
}

static int __cdecl music_db_get_card_default_list(int limit, int *results)
{
    return music_db_filtered_list(__func__, limit, results, filter_func_card_default);
}

// int __cdecl music_db_get_jukebox_list() {
//     log_body_warning("ultimate", "music_db_get_jukebox_list: not
//     implemented"); return 0;
// }

static enum music_load_res music_load_individual(int index, void *node)
{
    if (index >= MAX_SONGS) {
        return MUSIC_LOAD_FULL;
    }

    music_db_entry_t *song = &music_db[index];

    property_node_refer(nullptr, node, "/version", PROP_TYPE_str, tmp, sizeof(tmp));
    song->version = strtoul(tmp, nullptr, 16);

    if (!song->version) {
        return MUSIC_LOAD_BAD_VER;
    }

    // sane defaults
    memset(song->genre_list, 0, sizeof(song->genre_list));
    song->music_id = -1;
    song->parent_id = -1;
    song->name_sort_id_j = -1;
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

    property_node_refer(nullptr, node, "/music_id", PROP_TYPE_s32, &song->music_id, 4);
    property_node_refer(nullptr, node, "/parent_id", PROP_TYPE_s32, &song->parent_id, 4);
    property_node_refer(nullptr, node, "/bpm_max", PROP_TYPE_float, &song->bpm_max, 4);
    property_node_refer(nullptr, node, "/bpm_min", PROP_TYPE_float, &song->bpm_min, 4);
    property_node_refer(nullptr, node, "/name_sort_id_j", PROP_TYPE_str, tmp, sizeof(tmp));
    song->name_sort_id_j = strtoul(tmp, nullptr, 16);
    property_node_refer(nullptr, node, "/music_type", PROP_TYPE_s32, &song->music_type, 4);

    // ultimate music db only has detail_level
    property_node_refer(nullptr, node, "/detail_level_bsc", PROP_TYPE_float, &song->detail_level_bsc, 4);
    property_node_refer(nullptr, node, "/detail_level_adv", PROP_TYPE_float, &song->detail_level_adv, 4);
    property_node_refer(nullptr, node, "/detail_level_ext", PROP_TYPE_float, &song->detail_level_ext, 4);

    property_node_refer(nullptr, node, "/pos_index", PROP_TYPE_s16, &song->pos_index, 2);
    property_node_refer(nullptr, node, "/is_default", PROP_TYPE_s32, &song->is_default, 4);
    property_node_refer(
        nullptr, node, "/is_card_default", PROP_TYPE_s32, &song->is_card_default, 4);
    property_node_refer(
        nullptr, node, "/is_offline_default", PROP_TYPE_s32, &song->is_offline_default, 4);
    property_node_refer(nullptr, node, "/is_hold", PROP_TYPE_s32, &song->is_hold, 4);
    property_node_refer(nullptr, node, "/index_start", PROP_TYPE_s32, &song->index_start, 4);
    property_node_refer(nullptr, node, "/step", PROP_TYPE_s32, &song->step, 4);
    property_node_refer(nullptr, node, "genre/pops", PROP_TYPE_u8, &song->genre_list[0], 1);
    property_node_refer(nullptr, node, "genre/anime", PROP_TYPE_u8, &song->genre_list[1], 1);
    property_node_refer(nullptr, node, "genre/socialmusic", PROP_TYPE_u8, &song->genre_list[2], 1);
    property_node_refer(nullptr, node, "genre/game", PROP_TYPE_u8, &song->genre_list[3], 1);
    property_node_refer(nullptr, node, "genre/classic", PROP_TYPE_u8, &song->genre_list[4], 1);
    property_node_refer(nullptr, node, "genre/original", PROP_TYPE_u8, &song->genre_list[5], 1);
    property_node_refer(nullptr, node, "genre/toho", PROP_TYPE_u8, &song->genre_list[6], 1);
    property_node_refer(nullptr, node, "/pack_id", PROP_TYPE_s32, &song->pack_id, 4);
    property_node_refer(nullptr, node, "/grouping_category", PROP_TYPE_str, tmp, sizeof(tmp));
    song->grouping_category = strtoul(tmp, nullptr, 16);
    property_node_refer(
        nullptr, node, "/title_name", PROP_TYPE_str, song->title_name, sizeof(song->title_name));

    if (song->music_id == 70000154 && !song->grouping_category) {
        song->grouping_category = 4736;
    }

    music_db_map[song->music_id] = song;

#ifdef DEBUG_CHECK_MUSIC_IFS_EXISTS
    struct avs_stat st;
    char path[256];
    int lstat;
    snprintf(
        path, sizeof(path), "/data/ifs_pack/d%d/%d_msc.ifs", song->music_id / 10, song->music_id);
    if ((lstat = avs_fs_lstat(path, &st)) <= 0) {
        log_warning("Missing song IFS file for ID %d (%s)", song->music_id, song->title_name);
    } else if (st.filesize < 256)
    { // deleted songs don't get deleted, but have their IFS file stubbed
        log_warning("Too-short song IFS file for ID %d (%s)", song->music_id, song->title_name);
    } else {
        // for extra extra debugging
        //log_misc("Song id %d (%s) exists at %s lstat %d with size %d", song->music_id, song->title_name, path, lstat, st.filesize);
    }
#endif

    return MUSIC_LOAD_OK;
}

static int music_db_entry_sorter(const void *_a, const void *_b)
{
    auto a = *reinterpret_cast<const music_db_entry_t *const *>(_a);
    auto b = *reinterpret_cast<const music_db_entry_t *const *>(_b);

    return stricmp(a->title_name, b->title_name);
}

static bool __cdecl music_db_initialize(void)
{
    // some features are not worth reimplementing because their results aren't
    // changed by the ultimate songs. This lets us use these functions easily.
    music_db_initialize_orig();

    log_body_info("ultimate", "music_db_initialize...");
    music_count = 0;
    memset(music_db, 0, sizeof(music_db));
    music_db_map.clear();

#ifdef DEBUG_CHECK_MUSIC_IFS_EXISTS
    HMODULE avs2_core_handle;
    if ((avs2_core_handle = GetModuleHandleA("avs2-core.dll")) == nullptr) {
        log_fatal("GetModuleHandle(\"avs2-core.dll\") failed: 0x%08lx", GetLastError());
    }
    if ((avs_fs_lstat = get_proc_address<decltype(avs_fs_lstat)>(
             avs2_core_handle, "XCgsqzn0000063")) == nullptr)
    {
        log_fatal("GetProcAddress(\"avs_fs_lstat\") failed: 0x%08lx", GetLastError());
    }
#endif

    void *prop = nullptr;
    void *prop_mem = nullptr;
    // void *mdb_mem = avs_gheap_allocate(0, MDB_XML_SIZE, 0);
    void *mdb_mem = malloc(MDB_XML_SIZE);
    // note: loading music_ulti.xml
    int f = XFileLoadCall("data/music_info/music_ulti.xml", 1, mdb_mem, MDB_XML_SIZE);
    if (f) {
        while (XFileIsBusy(f)) {
            avs_thread_delay(4, 0);
        }
        int r = XFileLoadFinish(f);
        GFAssert(r);

        // prop_mem = avs_gheap_allocate(0, MDB_XML_SIZE, 0);
        prop_mem = malloc(MDB_XML_SIZE);
        prop = property_mem_read(mdb_mem, r, 23 | 0x1000u, prop_mem, MDB_XML_SIZE);
    }

    free(mdb_mem);
    // if(mdb_mem) {
    //     avs_gheap_free(mdb_mem);
    // }

    GFAssert(prop != nullptr);

    property_clear_error(prop);
    void *body = property_search(prop, 0, "/music_data/body");

    void *song = property_node_traversal(body, TRAVERSE_FIRST_CHILD);
    for (; song != nullptr; song = property_node_traversal(song, TRAVERSE_NEXT_SIBLING)) {
        if (music_load_individual(music_count, song) == MUSIC_LOAD_OK)
            ++music_count;
    }

    if (prop) {
        property_destroy(prop);
        // avs_gheap_free(prop_mem);
    }
    free(prop_mem);

    log_body_info("ultimate", "Loaded %d songs into music db", music_count);

    music_db_entry_t *sorted[MAX_SONGS];
    for (int i = 0; i < music_count; i++) {
        sorted[i] = &music_db[i];
    }

    // I could use avs_qsort here but what's the point when the stdlib is
    // guaranteed to be sane
    qsort(sorted, music_count, sizeof(music_db_entry_t *), music_db_entry_sorter);

    for (int i = 0; i < music_count; i++) {
        sorted[i]->name_sort_id_j = i;
    }

    return true;
}

// bool __cdecl music_db_finalize() {
//     return true;
// }

// bool __cdecl music_db_reset_using_datapackage(int a1) {
//     return true;
// }

// int __cdecl music_db_dbg_get_all_list() {
//     log_body_warning("ultimate", "music_db_dbg_get_all_list: not
//     implemented"); return 0;
// }

// int __cdecl music_db_dot_array_to_music_bar() {
//     log_body_warning("ultimate", "music_db_dot_array_to_music_bar: not
//     implemented"); return 0;
// }

static float __cdecl music_db_get_bpm(int id)
{
    music_db_entry_t *song = music_from_id(id);
    return song ? song->bpm_max : 0.0;
}

static float __cdecl music_db_get_bpm_min(int id)
{
    music_db_entry_t *song = music_from_id(id);
    return song ? song->bpm_min : -1.0;
}

// get_default_x: returns the song to play first? No patch needed

// int __cdecl music_db_get_default_id() {
//     log_body_warning("ultimate", "music_db_get_default_id: not implemented");
//     return 0;
// }

// int __cdecl music_db_get_default_id_by_genre() {
//     log_body_warning("ultimate", "music_db_get_default_id_by_genre: not
//     implemented"); return 0;
// }

// int __cdecl music_db_get_default_id_by_mode() {
//     log_body_warning("ultimate", "music_db_get_default_id_by_mode: not
//     implemented"); return 0;
// }

static char *__cdecl music_db_get_genre_list(int id)
{
    music_db_entry_t *song = music_from_id(id);
    GFAssert(song != nullptr); // this will crash if it's nullptr anyway, so may as well
                               // report cleanly

    return song->genre_list;
}

static uint64_t __cdecl music_db_get_grouping_category_list(int id)
{
    music_db_entry_t *song = music_from_id(id);
    return song ? song->grouping_category : 0;
}

static int __cdecl music_db_get_index_start(int id)
{
    music_db_entry_t *song = music_from_id(id);
    return song ? song->index_start : -1;
}

static uint8_t __cdecl music_db_get_level(int id, uint8_t difficulty)
{
    music_db_entry_t *song = music_from_id(id);
    if (!song) {
        return 1;
    }

    // ultimate music db only has detail_level
    switch (difficulty) {
        case 0:
            return static_cast<uint8_t>(song->detail_level_bsc);
        case 1:
            return static_cast<uint8_t>(song->detail_level_adv);
        case 2:
            return static_cast<uint8_t>(song->detail_level_ext);
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
//         log_body_fatal("ultimate", "music_db_get_level(%d, %d) -> %d", id,
//         difficulty, ret);
//     } else {
//         log_body_misc("ultimate", "music_db_get_level(%d, %d) -> %d", id,
//         difficulty, ret);
//     }

//     return ret;
// }

// returns the fractional part of levels, ie 9.4 -> 4
static uint8_t __cdecl music_db_get_level_detail(int id, uint8_t difficulty)
{
    // real code has handling to ignore level < 9, but the extra function
    // music_db_is_displayable_level_detail removes the need for it

    music_db_entry_t *song = music_from_id(id);
    if (!song) {
        return 0;
    }

    float diff;
    switch (difficulty) {
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

    return static_cast<uint8_t>(round(diff * 10.0)) % 10;
}

static int __cdecl music_db_get_music_name_head_index(int id)
{
    music_db_entry_t *song = music_from_id(id);
    // the bottom 12 bits are the sort order inside the category
    // the upper 20 bits (of which 10 are used) are the first letter of the
    // sort, with a single bit set ie (upper = 1<<letter)
    // this returns just that main sort category

    // specifically, these are the exact upper 20 bit assignments
    // 'ア': 0, 'イ': 0, 'ウ': 0, 'エ': 0, 'オ': 0, 'ン': 0,
    // 'カ': 1, 'キ': 1, 'ク': 1, 'ケ': 1, 'コ': 1,
    // 'サ': 2, 'シ': 2, 'ス': 2, 'セ': 2, 'ソ': 2,
    // 'タ': 3, 'チ': 3, 'ツ': 3, 'テ': 3, 'ト': 3,
    // 'ナ': 4, 'ニ': 4, 'ネ': 4, 'ノ': 4,
    // 'ハ': 5, 'ヒ': 5, 'フ': 5, 'ヘ': 5, 'ホ': 5,
    // 'マ': 6, 'ミ': 6, 'ム': 6, 'メ': 6, 'モ': 6,
    // 'ヤ': 7, 'ユ': 7, 'ヨ': 7,
    // 'ラ': 8, 'リ': 8, 'ル': 8, 'レ': 8, 'ロ': 8,
    // 'ワ': 9,

    return song ? song->name_sort_id_j >> 12 : 0;
}

static int __cdecl music_db_get_music_name_index(int id)
{
    music_db_entry_t *song = music_from_id(id);
    return song ? song->name_sort_id_j : 0;
}

static int __cdecl music_db_get_parent_music_id(int id)
{
    music_db_entry_t *song = music_from_id(id);
    return song ? song->parent_id : 0;
}

static uint8_t *__cdecl music_db_get_permitted_music_flag()
{
    static uint8_t flags[FLAG_LEN];
    memset(flags, -1, sizeof(flags));
    return flags;
}

static int16_t __cdecl music_db_get_pos_index(int id)
{
    music_db_entry_t *song = music_from_id(id);
    return song ? song->pos_index : -1;
}

// bool __cdecl music_db_is_all_yellow() {
//     log_body_warning("ultimate", "music_db_is_all_yellow: not implemented");
//     return 0;
// }

// bool __cdecl music_db_is_displayable_level_detail() {
//     log_body_warning("ultimate", "music_db_is_displayable_level_detail: not
//     implemented"); return 0;
// }

static bool __cdecl music_db_is_exists_table(int id)
{
    music_db_entry_t *song = music_from_id(id);
    return song != nullptr;
}

static int version_bit_count(int id)
{
    music_db_entry_t *song = music_from_id(id);
    if (!song) {
        return 0;
    }

    int bits = 0;
    while (!((1 << bits) & song->version)) {
        if (++bits >= 14) {
            break;
        }
    }

    return bits;
}

static bool __cdecl music_db_is_exists_version_from_ver1(int id)
{
    int bits = version_bit_count(id);
    return bits == 0 || bits == 1;
}

static bool __cdecl music_db_is_exists_version_from_ver2(int id)
{
    int bits = version_bit_count(id);
    return bits == 2 || bits == 3;
}

static bool __cdecl music_db_is_exists_version_from_ver3(int id)
{
    int bits = version_bit_count(id);
    return bits == 4 || bits == 5;
}

static bool __cdecl music_db_is_exists_version_from_ver4(int id)
{
    int bits = version_bit_count(id);
    return bits == 6 || bits == 7;
}

static bool __cdecl music_db_is_exists_version_from_ver5(int id)
{
    int bits = version_bit_count(id);
    return bits == 8 || bits == 9;
}

static bool __cdecl music_db_is_exists_version_from_ver5_5(int id)
{
    return version_bit_count(id) == 9;
}

static bool __cdecl music_db_is_exists_version_from_ver6(int id)
{
    return version_bit_count(id) == 10;
}

static bool __cdecl music_db_is_exists_version_from_ver7(int id)
{
    return version_bit_count(id) == 11;
}

static bool __cdecl music_db_is_exists_version_from_ver8(int id)
{
    return version_bit_count(id) == 12;
}

static bool __cdecl music_db_is_exists_version_from_ver9(int id)
{
    int bits = version_bit_count(id);
    return bits == 13 || bits == 14;
}

static bool __cdecl music_db_is_hold_marker(int id)
{
    music_db_entry_t *song = music_from_id(id);
    return song ? song->is_hold : 0;
}

static bool __cdecl music_db_is_matched_select_type(uint8_t type, int id, uint8_t difficulty)
{
    int8_t level = static_cast<int8_t>(music_db_get_level(id, difficulty));

    switch (type) {
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

/*
bool __cdecl music_db_is_matching_select() {
    log_body_warning("ultimate", "music_db_is_matching_select: not implemented");
    return 0;
}

bool __cdecl music_db_is_nearly_excellent() {
    log_body_warning("ultimate", "music_db_is_nearly_excellent: not implemented");
    return 0;
    }

bool __cdecl music_db_is_nearly_fullcombo() {
    log_body_warning("ultimate", "music_db_is_nearly_fullcombo: not implemented");
    return 0;
}

// this one's a bit complex, just do the best we can with the base data
bool __cdecl music_db_is_new() {
    log_body_warning("ultimate", "music_db_is_new: not implemented");
    return 0;
}

bool __cdecl music_db_is_no_gray() {
    log_body_warning("ultimate", "music_db_is_no_gray: not implemented");
    return 0;
}
*/

static bool __cdecl music_db_is_permitted(int id)
{
    return music_db_is_exists_table(id);
}

// this is used along with the hot_music_list bitfield to determine
// pickup/common in jubility rankings. You can't just return every song or the
// common folder will be empty. Every *other* place it appears should just
// return true all the time as an "unlock all" patch.
// hot_music_list THANKFULLY is one of two lists that comes in from .data, every
// other call uses a stack variable. By detecting this we can only perform
// filtering when we absolutely have to.
static bool __cdecl music_db_is_possession_for_contained_music_list(uint8_t flags[FLAG_LEN], int id)
{
    static uint8_t *data_start = nullptr;
    static uint8_t *data_end = nullptr;

    static uint8_t *hot_music = nullptr;

    if (data_start == nullptr) {
        auto dll_dos = reinterpret_cast<PIMAGE_DOS_HEADER>(GetModuleHandle("jubeat.dll"));
        GFAssert(dll_dos->e_magic == IMAGE_DOS_SIGNATURE);

        auto nt_headers = reinterpret_cast<PIMAGE_NT_HEADERS>(
            reinterpret_cast<uint8_t *>(dll_dos) + dll_dos->e_lfanew);
        auto section_count = nt_headers->FileHeader.NumberOfSections;

        // iterate sections
        PIMAGE_SECTION_HEADER section_header = IMAGE_FIRST_SECTION(nt_headers);
        for (size_t i = 0; i < section_count; section_header++, i++) {
            auto name = reinterpret_cast<const char *>(section_header->Name);

            if (!str_eq(".data", name)) {
                continue;
            }

            data_start = reinterpret_cast<uint8_t *>(
                reinterpret_cast<uintptr_t>(dll_dos) + section_header->VirtualAddress);
            data_end = data_start + section_header->Misc.VirtualSize;

            log_body_misc("ultimate", ".data found from %p to %p", data_start, data_end);

            break;
        }

        GFAssert(data_start != nullptr);
        GFAssert(data_end != nullptr);
    }

    if (hot_music == nullptr && flags >= data_start && flags < data_end &&
        // hot_music lives at a significantly larger offset than the others
        (flags - data_start) > 0x1000000)
    {
        hot_music = flags;
        log_body_misc("ultimate", "hot_music found at %p", hot_music);
    }

    // log_body_misc("ultimate", "%s(%p, %d)", __func__, flags, id);

    /*
    static void* last_flags = nullptr;
    if (flags != last_flags) {
        last_flags = flags;

        for (int i = 0; i < FLAG_LEN; i += 8) {
            log_body_misc("ultimate", "%s %d: %X %X %X %X %X %X %X %X",
                __func__,
                i,
                flags[i],
                flags[i + 1],
                flags[i + 2],
                flags[i + 3],
                flags[i + 4],
                flags[i + 5],
                flags[i + 6],
                flags[i + 7]
            );
        }
    }
    */

    /*
    static uint8_t all_ff[FLAG_LEN] = {0};
    if (all_ff[0] == 0) {
        memset(all_ff, 0xff, sizeof(all_ff));
    }
    */

    // where we just unlock things
    if (flags != hot_music) {
        // if(memcmp(all_ff, flags, FLAG_LEN) == 0) {
        return music_db_is_exists_table(id);
    }

    // jubility stuff
    struct music_db_entry_t *song = music_from_id(id);
    if (!song) {
        return false;
    }

    if (song->pos_index < 0 || song->pos_index >= MAX_SONGS_STOCK) {
        return 0;
    }

    size_t flag_byte = song->pos_index / 8;
    size_t flag_bit = song->pos_index % 8;

    return (flags[flag_byte] & (1 << flag_bit)) != 0;
}

/*
// ID test
bool __cdecl music_db_is_random_or_matching_select() {
    log_body_warning("ultimate", "music_db_is_random_or_matching_select: not implemented");
    return 0;
}

bool __cdecl music_db_is_random_select() {
    log_body_warning("ultimate", "music_db_is_random_select: not implemented");
    return 0;
}

int __cdecl music_db_music_bar_to_dot_array() {
    log_body_warning("ultimate", "music_db_music_bar_to_dot_array: not implemented");
    return 0;
}

int __cdecl music_db_set_default_add_music_flag(uint8_t flags[FLAG_LEN]) {
    return 0;
}

int __cdecl music_db_set_flag_equivalent_for_music_id(void *a1, unsigned int flag, int value) {
    // I *think* this can do nothing
    log_body_warning("ultimate", "music_db_set_flag_equivalent_for_music_id(%p, %d, %d): not implemented", a1, flag, value);
    return 0;
}

int __cdecl music_db_set_permitted_music_flag(uint8_t flags[FLAG_LEN]) {
    return 0;
}
*/

// these functions handily replace all the music_record functions because
// they call GFHashMapKeyToValue before checking size limits
static int music_record_count;
static uint8_t music_records[MAX_SONGS][2112]; // dll says 2112 bytes each
static robin_hood::unordered_map<int, void *> music_record_map;

static void *__cdecl GFHashMapCreate(void *mem, int mem_sz, int max_elems)
{
    log_body_info("ultimate", "hooked GFHashMapCreate");

    if (music_count == 0) {
        log_body_fatal("ultimate", "GFHashMapCreate called before mdb load, cannot continue");
    }

    memset(music_records, 0, sizeof(music_records));
    music_record_count = 0;
    music_record_map.clear();

    // pre-populate the hashmap to avoid issues down the track
    for (int i = 0; i < music_count; i++) {
        music_record_map[music_db[i].music_id] = music_records[music_record_count++];
    }

    return &music_record_map;
}

static void __cdecl GFHashMapRegist(void *map, int key, void *val)
{
    log_body_fatal("ultimate", "GFHashMapRegist should not be called if patches worked");
}

static void *__cdecl GFHashMapKeyToValue(void *map, int key)
{
    if (music_count == 0) {
        log_body_fatal("ultimate", "GFHashMapKeyToValue called before mdb load, cannot continue");
    }

    auto search = music_record_map.find(key);
    if (search == music_record_map.end()) {
        if (music_record_count >= MAX_SONGS) {
            log_body_fatal("ultimate", "music records full even with gratuitous buffer");
        }

        // insert new, wasn't in the music db but maybe tutorial or something
        // log_body_warning("ultimate", "music_record added unknown ID %d", key);
        music_record_map[key] = music_records[music_record_count];
        return music_records[music_record_count++];
    } else {
        return search->second;
    }
}

// this has potential for failure, but it works for now
static int music_record_iter;

static void __cdecl GFHashMapRewindEntryList(void *map)
{
    music_record_iter = 0;
}

static bool GFHashMapGetEntryList(void *map, int *key, void **val)
{
    if (music_record_iter >= music_count) {
        return false;
    }

    *key = music_db[music_record_iter].music_id;
    *val = music_records[music_record_iter];
    music_record_iter++;

    return true;
}

/*
// internal 100 element list?
int __cdecl music_db_set_select_history_list() {
    log_body_warning("ultimate", "music_db_set_select_history_list: not implemented");
    return 0;
}

int __cdecl music_bonus_get_bonus_music() {
    log_body_warning("ultimate", "music_bonus_get_bonus_music: not
    implemented"); return 0;
}
int __cdecl music_bonus_is_bonus_music() {
    log_body_warning("ultimate", "music_bonus_is_bonus_music: not
    implemented"); return 0;
}
// only_now functions: some internal list, only 12 elements long. Passthrough.
int __cdecl music_only_now_get_count() {
    log_body_warning("ultimate", "music_only_now_get_count: not
    implemented"); return 0;
}
int __cdecl music_only_now_get_etime() {
    log_body_warning("ultimate", "music_only_now_get_etime: not
    implemented"); return 0;
}
int __cdecl music_only_now_get_music_id() {
    log_body_warning("ultimate", "music_only_now_get_music_id: not
    implemented"); return 0;
}
bool __cdecl music_only_now_initialize() {
    return true;
}
int __cdecl music_record_add_clear_count() {
    log_body_warning("ultimate", "music_record_add_clear_count: not
    implemented"); return 0;
}
int __cdecl music_record_add_excellent_count() {
    log_body_warning("ultimate", "music_record_add_excellent_count: not
    implemented"); return 0;
}
int __cdecl music_record_add_full_combo_count() {
    log_body_warning("ultimate", "music_record_add_full_combo_count: not
    implemented"); return 0;
}
int __cdecl music_record_add_play_count() {
    log_body_warning("ultimate", "music_record_add_play_count: not
    implemented"); return 0;
}
int __cdecl music_record_clear_context() {
    log_body_warning("ultimate", "music_record_clear_context: not
    implemented"); return 0;
}
int __cdecl music_record_get_best_music_rate() {
    log_body_warning("ultimate", "music_record_get_best_music_rate: not
    implemented"); return 0;
}
int __cdecl music_record_get_best_score() {
    log_body_warning("ultimate", "music_record_get_best_score: not
    implemented"); return 0;
}
int __cdecl music_record_get_clear_count() {
    log_body_warning("ultimate", "music_record_get_clear_count: not
    implemented"); return 0;
}
int __cdecl music_record_get_clear_flag() {
    log_body_warning("ultimate", "music_record_get_clear_flag: not
    implemented"); return 0;
}
int __cdecl music_record_get_excellent_count() {
    log_body_warning("ultimate", "music_record_get_excellent_count: not
    implemented"); return 0;
}
int __cdecl music_record_get_full_combo_count() {
    log_body_warning("ultimate", "music_record_get_full_combo_count: not
    implemented"); return 0;
}
int __cdecl music_record_get_jubility() {
    log_body_warning("ultimate", "music_record_get_jubility: not
    implemented"); return 0;
}
int __cdecl music_record_get_music_bar() {
    log_body_warning("ultimate", "music_record_get_music_bar: not
    implemented"); return 0;
}
int __cdecl music_record_get_play_count() {
    log_body_warning("ultimate", "music_record_get_play_count: not
    implemented"); return 0;
}
int __cdecl music_record_get_sequence_record_set() {
    log_body_warning("ultimate", "music_record_get_sequence_record_set: not
    implemented"); return 0;
}
int __cdecl music_record_get_total_best_score() {
    log_body_warning("ultimate", "music_record_get_total_best_score: not
    implemented"); return 0;
}
int __cdecl music_record_is_all_played() {
    log_body_warning("ultimate", "music_record_is_all_played: not
    implemented"); return 0;
}
int __cdecl music_record_is_all_yellow() {
    log_body_warning("ultimate", "music_record_is_all_yellow: not
    implemented"); return 0;
}
int __cdecl music_record_is_any_played() {
    log_body_warning("ultimate", "music_record_is_any_played: not
    implemented"); return 0;
}
int __cdecl music_record_is_cleared() {
    log_body_warning("ultimate", "music_record_is_cleared: not implemented");
    return 0;
}
int __cdecl music_record_is_excellent() {
    log_body_warning("ultimate", "music_record_is_excellent: not
    implemented"); return 0;
}
int __cdecl music_record_is_full_combo() {
    log_body_warning("ultimate", "music_record_is_full_combo: not
    implemented"); return 0;
}
int __cdecl music_record_is_no_gray() {
    log_body_warning("ultimate", "music_record_is_no_gray: not implemented");
    return 0;
}
int __cdecl music_record_is_played(int id, uint8_t a2, int a3) {
    log_body_warning("ultimate", "music_record_is_played(%d, %d, %d): not
    implemented",id,a2,a3); return 0;
}
int __cdecl music_record_merge_music_bar() {
    log_body_warning("ultimate", "music_record_merge_music_bar: not
    implemented"); return 0;
}
int __cdecl music_record_set_best_music_rate() {
    log_body_warning("ultimate", "music_record_set_best_music_rate: not
    implemented"); return 0;
}
int __cdecl music_record_set_best_score() {
    log_body_warning("ultimate", "music_record_set_best_score: not
    implemented"); return 0;
}
int __cdecl music_record_set_clear_count() {
    log_body_warning("ultimate", "music_record_set_clear_count: not
    implemented"); return 0;
}
int __cdecl music_record_set_clear_flag() {
    log_body_warning("ultimate", "music_record_set_clear_flag: not
    implemented"); return 0;
}
int __cdecl music_record_set_excellent_count() {
    log_body_warning("ultimate", "music_record_set_excellent_count: not
    implemented"); return 0;
}
int __cdecl music_record_set_full_combo_count() {
    log_body_warning("ultimate", "music_record_set_full_combo_count: not
    implemented"); return 0;
}
int __cdecl music_record_set_jubility() {
    log_body_warning("ultimate", "music_record_set_jubility: not
    implemented"); return 0;
}
int __cdecl music_record_set_play_count() {
    log_body_warning("ultimate", "music_record_set_play_count: not
    implemented"); return 0;
}
shareable - tentative "not needed"
int __cdecl music_shareable_add_shareable_music() {
    log_body_warning("ultimate", "music_shareable_add_shareable_music: not
    implemented"); return 0;
}
bool __cdecl music_shareable_initialize() {
    return 1;
}
int __cdecl music_shareable_is_shareable_music() {
    log_body_warning("ultimate", "music_shareable_is_shareable_music: not
    implemented"); return 1;
}
void __cdecl music_shareable_set_flag(uint8_t flags[FLAG_LEN]) {
}

void *__cdecl music_texture_BlackJacket_GetInstance() {
    return nullptr;
}
int __cdecl music_texture_BlackJacket_ReadXmlNode() {
    return 0;
}
int __cdecl music_bonus_weekly_clear() {
    log_body_warning("ultimate", "music_bonus_weekly_clear: not
    implemented"); return 0;
}
int __cdecl music_bonus_weekly_music_bonus_get_target_music_info() {
    log_body_warning("ultimate",
    "music_bonus_weekly_music_bonus_get_target_music_info: not implemented");
    return 0;
}
int __cdecl music_bonus_weekly_music_bonus_weekly_is_target_music() {
    log_body_warning("ultimate",
    "music_bonus_weekly_music_bonus_weekly_is_target_music: not
    implemented"); return 0;
}
int __cdecl music_new_clear() {
    log_body_warning("ultimate", "music_new_clear: not implemented");
    return 0;
}
int __cdecl music_new_get_list() {
    log_body_warning("ultimate", "music_new_get_list: not implemented");
    return 0;
}
int __cdecl music_new_read_xml_node() {
    log_body_warning("ultimate", "music_new_read_xml_node: not implemented");
    return 0;
}
*/

// intentionally ignored:
// music_db_is_tutorial

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
