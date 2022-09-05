#include "categories.h"
#include "music_db.h"

enum custom_sort_id: uint32_t {
    SORT_CUSTOM_ULTIMATE = SORT_DEFAULT_MAX_ID,
    SORT_CUSTOM_OMNIMIX,
    SORT_CUSTOM_JUBEAT_PLUS,
    SORT_CUSTOM_JUBEAT_2020,
    SORT_CUSTOM_JUKEBEAT,
};

static bool __cdecl custom_filter_omnimix(unsigned music_id, int diff, uint8_t level) {
    (void) diff;
    (void) level;

    music_db_entry_t *music = music_from_id(music_id);

    return music->ultimate_list_omnimix;
}

static bool __cdecl custom_filter_jubeat_plus(unsigned music_id, int diff, uint8_t level) {
    (void) diff;
    (void) level;

    music_db_entry_t *music = music_from_id(music_id);

    return music->ultimate_list_jubeat_plus;
}

static bool __cdecl custom_filter_jubeat_2020(unsigned music_id, int diff, uint8_t level) {
    (void) diff;
    (void) level;

    music_db_entry_t *music = music_from_id(music_id);

    return music->ultimate_list_jubeat_2020;
}

static bool __cdecl custom_filter_jukebeat(unsigned music_id, int diff, uint8_t level) {
    (void) diff;
    (void) level;

    music_db_entry_t *music = music_from_id(music_id);

    return music->ultimate_list_jukebeat;
}

const std::vector<category_hierarchy_t> extra_category_hierarchy = {
    {SORT_CUSTOM_ULTIMATE,    SORT_ROOT,            NULL,                      "SMM_T0950_JA.png", NULL, NULL},
    {SORT_CUSTOM_OMNIMIX,     SORT_CUSTOM_ULTIMATE, custom_filter_omnimix,     "SMM_T0960_JA.png", NULL, NULL},
    {SORT_CUSTOM_JUBEAT_PLUS, SORT_CUSTOM_ULTIMATE, custom_filter_jubeat_plus, "SMM_T0961_JA.png", NULL, NULL},
    {SORT_CUSTOM_JUBEAT_2020, SORT_CUSTOM_ULTIMATE, custom_filter_jubeat_2020, "SMM_T0962_JA.png", NULL, NULL},
    {SORT_CUSTOM_JUKEBEAT,    SORT_CUSTOM_ULTIMATE, custom_filter_jukebeat,    "SMM_T0963_JA.png", NULL, NULL},
};

const std::vector<category_listing_t> extra_category_layout = {
    // Ultimate invalidates a lot of existing categories and adds our own.
    // Custom categories are prepended to the list, so we can totally replace
    // the root by fully specifying it here
    {SORT_ROOT, 4, {
        // SORT_ROOT in SORT_ROOT is the "Select a category" icon
        {SORT_ROOT, SORT_CUSTOM_ULTIMATE, SORT_ALL},
        {SORT_CATEGORY_VERSION, SORT_CATEGORY_GENRE, SORT_CATEGORY_LEVEL},
        {SORT_CATEGORY_NAME, SORT_CATEGORY_ARTIST, SORT_HOLD},
        {SORT_TUNE_RUN, SORT_THIS_WEEK_RECOMMENDED, SORT_TARGET_MUSIC},
    }},
    {SORT_CUSTOM_ULTIMATE, 3, {
        {}, // leave first column blank
        {SORT_CUSTOM_OMNIMIX, SORT_CUSTOM_JUBEAT_PLUS, SORT_CUSTOM_JUBEAT_2020},
        {SORT_CUSTOM_JUKEBEAT},
    }},
};
