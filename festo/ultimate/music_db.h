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

bool __cdecl music_db_get_sequence_filename(void *a1, void *a2, int music_id, uint8_t seq);
bool __cdecl music_db_get_sound_filename(void *a1, void *a2, int music_id, uint8_t seq);

extern bool __cdecl (*music_db_initialize_orig)();

//bool __cdecl music_db_finalize();
bool __cdecl music_db_initialize();
//bool __cdecl music_db_reset_using_datapackage(int a1);
//int __cdecl music_db_dbg_get_all_list();
//int __cdecl music_db_dot_array_to_music_bar();
int __cdecl music_db_get_default_list(int limit, int* results);
int __cdecl music_db_get_offline_default_list(int limit, int* results);
int __cdecl music_db_get_all_permitted_list(int limit, int *results);
int __cdecl music_db_get_possession_list(uint8_t flags[FLAG_LEN], int limit, int *results);
int __cdecl music_db_get_card_default_list(int limit, int *results);
//int __cdecl music_db_get_jukebox_list();
float __cdecl music_db_get_bpm(int id);
float __cdecl music_db_get_bpm_min(int id);
// int __cdecl music_db_get_default_id();
// int __cdecl music_db_get_default_id_by_genre();
// int __cdecl music_db_get_default_id_by_mode();
char* __cdecl music_db_get_genre_list(int id);
uint64_t __cdecl music_db_get_grouping_category_list(int id);
int __cdecl music_db_get_index_start(int id);
uint8_t __cdecl music_db_get_level(int id, uint8_t difficulty);
// hook this instead if you're having issues
//uint8_t __cdecl music_db_get_level_logged(int id, uint8_t difficulty);
uint8_t __cdecl music_db_get_level_detail(int id, uint8_t difficulty);
int __cdecl music_db_get_music_name_head_index(int id);
int __cdecl music_db_get_music_name_index(int id);
int __cdecl music_db_get_parent_music_id(int id);
uint8_t *__cdecl music_db_get_permitted_music_flag();
int16_t __cdecl music_db_get_pos_index(int a1);
//bool __cdecl music_db_is_all_yellow();
//bool __cdecl music_db_is_displayable_level_detail();
bool __cdecl music_db_is_exists_table(int id);
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
bool __cdecl music_db_is_hold_marker(int id);
bool __cdecl music_db_is_matched_select_type(uint8_t type, int id, uint8_t difficulty);
//bool __cdecl music_db_is_matching_select();
// bool __cdecl music_db_is_nearly_excellent();
// bool __cdecl music_db_is_nearly_fullcombo();
//bool __cdecl music_db_is_new();
//bool __cdecl music_db_is_no_gray();
bool __cdecl music_db_is_permitted(int id);
bool __cdecl music_db_is_possession_for_contained_music_list(uint8_t flags[FLAG_LEN], int a2);
//bool __cdecl music_db_is_random_or_matching_select();
//bool __cdecl music_db_is_random_select();
//int __cdecl music_db_music_bar_to_dot_array();
// int __cdecl music_db_set_default_add_music_flag(uint8_t flags[FLAG_LEN]);
// int __cdecl music_db_set_flag_equivalent_for_music_id(void *a1, unsigned int flag, int value);
// int __cdecl music_db_set_permitted_music_flag(uint8_t flags[FLAG_LEN]);
//int __cdecl music_db_set_select_history_list();

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

void __cdecl *mem_set(void *s, int c, size_t n);

void GFHashMapRegist(void *map, int key, void *val);
void* GFHashMapCreate(void *mem, int mem_sz, int max_elems);
void* GFHashMapKeyToValue(void *map, int key);
bool GFHashMapGetEntryList(void *map, int *key, void **val);
void GFHashMapRewindEntryList(void *map);

#ifdef __cplusplus
};
#endif
