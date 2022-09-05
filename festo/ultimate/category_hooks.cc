// #include <stdbool.h>
// #include <stdint.h>

#define LOG_MODULE "ultimate::category_hooks"

// clang-format off
#include <windows.h>
#include <psapi.h>
// clang-format on

#include <unordered_map>

#include "category_hooks.h"

#include "pattern/pattern.h"

#include "util/log.h"
#include "util/defs.h"
#include "util/patch.h"
#include "util/robin_hood.h"
#include "util/mem.h"
#include "util/x86.h"

#include "../common/festo.h"

#include "MinHook.h"

enum env_lang {
    ENV_LANG_A = 5,
    ENV_LANG_C = 4,
    ENV_LANG_EU = 1,
    ENV_LANG_K = 3,
    ENV_LANG_J = 2
};

static bool init_done;

static robin_hood::unordered_map<uint32_t, uint32_t> extend_category_hierarchy;
static robin_hood::unordered_map<uint32_t, music_sort_function> music_sorters;

typedef struct {
    uint32_t category_id;
    enum env_lang lang;
    const char *tex_name;
} scroll_category_map_t;

static_assert(sizeof(scroll_category_map_t) == 12);

// existing entries taken from ScrollPanelCategorySetup - much much simpler
static std::vector<scroll_category_map_t> category_textures = {
    #include "category_textures_default.h"
};

static uint32_t (__fastcall *orig_category_select_list_get_parent_type)(uint32_t category);
static uint32_t __fastcall category_select_list_get_parent_type(uint32_t category) {
    // log_info("category_select_list_get_parent_type(%d)", category);
    auto parent = extend_category_hierarchy.find(category);
    if(parent == extend_category_hierarchy.end()) {
        return orig_category_select_list_get_parent_type(category);
    } else {
        return parent->second;
    }
}

static music_sort_function (__fastcall *orig_get_music_filter_func)(uint32_t sort_id);
static music_sort_function __fastcall get_music_filter_func(uint32_t sort_id) {
    auto sorter = music_sorters.find(sort_id);
    if(sorter == music_sorters.end()) {
        return orig_get_music_filter_func(sort_id);
    } else {
        return sorter->second;
    }
}

// required to correctly populate the array
#define ORIG_LISTING_SZ 9

static std::vector<category_listing_t> hook_category_listing;

void category_hooks_add_category_definitions(std::vector<category_hierarchy_t> categories)
{
    log_assert(init_done == false);

    for(auto &cat : categories) {
        extend_category_hierarchy[cat.id] = cat.parent;

        if(cat.sort_fn) {
            music_sorters[cat.id] = cat.sort_fn;
        }

        auto tex_k = cat.tex_name_lang_k ? cat.tex_name_lang_k : cat.tex_name_lang_j;
        auto tex_a = cat.tex_name_lang_a ? cat.tex_name_lang_a : cat.tex_name_lang_j;
        category_textures.push_back({cat.id, ENV_LANG_J, cat.tex_name_lang_j});
        category_textures.push_back({cat.id, ENV_LANG_K, tex_k});
        category_textures.push_back({cat.id, ENV_LANG_A, tex_a});
    }
}

void category_hooks_add_category_layouts(std::vector<category_listing_t> listings)
{
    hook_category_listing.insert(hook_category_listing.end(), listings.begin(), listings.end());
}

void write_u32(HANDLE process, void *target, uint32_t replace) {
    uint32_t orig = *reinterpret_cast<uint32_t*>(target);
    log_info("write_u32 %08X -> %08X", orig, replace);
    memory_write(process, target, {U32_TO_CONST_BYTES_LE(replace)});
}

void category_hooks_init(HANDLE process, const MODULEINFO &jubeat_info) {
    log_assert(init_done == false);
    log_info("jubeat.dll base: %p", jubeat_info.lpBaseOfDll);

    // hook: category_select_list_get_parent_type, use extend_category_hierarchy
    auto cat_select = find_pattern_checked(
        "category_select_list_get_parent_type", jubeat_info,
        {0x55, 0x8B, 0xEC, 0x81, 0xEC, 0xC8, 0x03, 0x00, 0x00, 0xA1, 0x08, 0x00, 0x2B, 0x10, 0x33, 0xC5, 0x89, 0x45, 0xFC, 0x56, 0x57},
        {   1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    0,    0,    0,    0,    1,    1,    1,    1,    1,    1,    1}
    );
    MH_CreateHook(cat_select,
        reinterpret_cast<void *>(category_select_list_get_parent_type),
        reinterpret_cast<void **>(&orig_category_select_list_get_parent_type));

    // hook: music_filter_func, return new sorters
    auto music_filter = find_pattern_checked(
        "get_music_filter_func", jubeat_info,
        {0x83, 0xC1, 0xFE, 0x83, 0xF9, 0x78, 0x0F}, {}
    );
    MH_CreateHook(music_filter,
        reinterpret_cast<void *>(get_music_filter_func),
        reinterpret_cast<void **>(&orig_get_music_filter_func));

    // grab the address of the original array (before relocations is 0x102B8300)
    uint8_t *orig_category_listing_use = find_pattern_checked(
        "orig_category_listing_use", jubeat_info,
        {0x39, 0x88, 0x00, 0x83, 0x2B, 0x10, 0x74, 0x16},
        {   1,    1,    0,    0,    0,    0,    1,    1}
    ) + 2;
    category_listing_t *orig_category_listing = *reinterpret_cast<category_listing_t**>(orig_category_listing_use);
    log_info("orig_category_listing at %p", orig_category_listing);

    // populate the hook array with the original data
    hook_category_listing.insert(hook_category_listing.end(), &orig_category_listing[0], &orig_category_listing[ORIG_LISTING_SZ]);

    // hook: the category array in category_handler_thing
    const uintptr_t category_listing_patch = reinterpret_cast<uintptr_t>(hook_category_listing.data());

    const struct patch_t category_map_length_patch {
        .name = "category map length",
        .pattern = {0x42, 0x3D, 0x08, 0x07, 0x00, 0x00},
        .data = {U32_TO_CONST_BYTES_LE(hook_category_listing.size() * sizeof(category_listing_t))},
        .data_offset = 2,
    };
    write_u32(process, orig_category_listing_use, category_listing_patch);
    // this is EXTREMELY lazy and fragile
    write_u32(process, orig_category_listing_use + 53, category_listing_patch+4);
    write_u32(process, orig_category_listing_use + 75, category_listing_patch+8);
    // and the size limit
    do_patch(process, jubeat_info, category_map_length_patch);

    // hook: the texture name array in ScrollPanelCategorySetup
    // original code accesses the lang element
    const uintptr_t category_textures_patch = reinterpret_cast<uintptr_t>(&category_textures[0].lang);
    const uintptr_t category_textures_patch2 = reinterpret_cast<uintptr_t>(&category_textures[0].tex_name);

    const struct patch_t category_array_patch {
        .name = "category array",
        .pattern = { 0x8D, 0x85, 0x1C, 0xF0, 0xFF, 0xFF },
        .data = { 0xB8, U32_TO_CONST_BYTES_LE(category_textures_patch), 0x90 },
        .data_offset = 0,
    };

    const struct patch_t category_array_patch_2 {
        .name = "category array 2",
        // mov    eax,DWORD PTR [ebp+eax*4-0xfe0]
        .pattern = { 0x8B, 0x84, 0x85, 0x20, 0xF0, 0xFF, 0xFF },
        // mov    eax,DWORD PTR [eax*4+category_textures_patch2]
        .data = { 0x8B, 0x04, 0x85, U32_TO_CONST_BYTES_LE(category_textures_patch2) },
        .data_offset = 0,
    };

    const struct patch_t category_count_patch {
        .name = "category array size",
        .pattern = {0x83, 0xC0, 0x0C, 0x81, 0xF9, 0x53, 0x01, 0x00, 0x00},
        .data = {U32_TO_CONST_BYTES_LE(category_textures.size())},
        .data_offset = 5,
    };
    do_patch(process, jubeat_info, category_array_patch);
    do_patch(process, jubeat_info, category_array_patch_2);
    do_patch(process, jubeat_info, category_count_patch);

    log_info("Hooked categories:");
    for(auto &cat: hook_category_listing) {
        log_info("  %d %d", cat.id, cat.column_count);
        auto cols = cat.columns;
        for(size_t j = 0; j < 16; j++) {
            log_info("    %d %d %d", cols[j][0],cols[j][1],cols[j][2]);
        }
    }

    init_done = true;
}

category_listing_t *category_hooks_get_listing(uint32_t id)
{
    log_assert(init_done == true);

    for(auto &cat : hook_category_listing) {
        if(cat.id == id) {
            return &cat;
        }
    }

    log_fatal("Couldn't find category %d with category_hooks_get_listing", id);
    return NULL;
}
