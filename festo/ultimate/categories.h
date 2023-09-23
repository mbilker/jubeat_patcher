#include "category_hooks.h"

extern const std::vector<category_hierarchy_t> extra_category_hierarchy;
extern const std::vector<category_listing_t> extra_category_layout;

extern const std::vector<grouping_textures_t> extra_group_textures;
uint32_t __fastcall category_group_fn_alphabet(enum group_type group_type, const music_info_for_grouping_t *info);
uint32_t __fastcall category_group_fn_genre_custom(enum group_type group_type, const music_info_for_grouping_t *info);
uint32_t __fastcall category_group_fn_version_custom(enum group_type group_type, const music_info_for_grouping_t *info);

int __cdecl version_sorter(const music_info_for_grouping_t *a, const music_info_for_grouping_t *b);
int __cdecl genre_sorter(const music_info_for_grouping_t *a, const music_info_for_grouping_t *b);
