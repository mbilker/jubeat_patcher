#define LOG_MODULE "ultimate::categories"

#include "categories.h"
#include "music_db.h"

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif

enum custom_sort_id: uint32_t {
    SORT_CUSTOM_ULTIMATE = SORT_DEFAULT_MAX_ID,
    SORT_CUSTOM_OMNIMIX,
    SORT_CUSTOM_JUBEAT_PLUS,
    SORT_CUSTOM_JUBEAT_2020,
    SORT_CUSTOM_JUKEBEAT,

    SORT_CUSTOM_A,
    SORT_CUSTOM_B,
    SORT_CUSTOM_C,
    SORT_CUSTOM_D,
    SORT_CUSTOM_E,
    SORT_CUSTOM_F,
    SORT_CUSTOM_G,
    SORT_CUSTOM_H,
    SORT_CUSTOM_I,
    SORT_CUSTOM_J,
    SORT_CUSTOM_K,
    SORT_CUSTOM_L,
    SORT_CUSTOM_M,
    SORT_CUSTOM_N,
    SORT_CUSTOM_O,
    SORT_CUSTOM_P,
    SORT_CUSTOM_Q,
    SORT_CUSTOM_R,
    SORT_CUSTOM_S,
    SORT_CUSTOM_T,
    SORT_CUSTOM_U,
    SORT_CUSTOM_V,
    SORT_CUSTOM_W,
    SORT_CUSTOM_X,
    SORT_CUSTOM_Y,
    SORT_CUSTOM_Z,
};

static bool __cdecl custom_filter_omnimix(unsigned music_id, int diff, uint8_t level) {
    music_db_entry_t *music = music_from_id(music_id);

    return music->ultimate_list_omnimix;
}

static bool __cdecl custom_filter_jubeat_plus(unsigned music_id, int diff, uint8_t level) {
    music_db_entry_t *music = music_from_id(music_id);

    return music->ultimate_list_jubeat_plus;
}

static bool __cdecl custom_filter_jubeat_2020(unsigned music_id, int diff, uint8_t level) {
    music_db_entry_t *music = music_from_id(music_id);

    return music->ultimate_list_jubeat_2020;
}

static bool __cdecl custom_filter_jukebeat(unsigned music_id, int diff, uint8_t level) {
    music_db_entry_t *music = music_from_id(music_id);

    return music->ultimate_list_jukebeat;
}

#define MAKE_ALPHABET_SORTER(letter, chr) \
    static bool __cdecl custom_filter_ ## letter(unsigned music_id, int diff, uint8_t level) { \
        music_db_entry_t *music = music_from_id(music_id); \
        return music->sort_name[0] == chr; \
    }

MAKE_ALPHABET_SORTER(a, 'a')
MAKE_ALPHABET_SORTER(b, 'b')
MAKE_ALPHABET_SORTER(c, 'c')
MAKE_ALPHABET_SORTER(d, 'd')
MAKE_ALPHABET_SORTER(e, 'e')
MAKE_ALPHABET_SORTER(f, 'f')
MAKE_ALPHABET_SORTER(g, 'g')
MAKE_ALPHABET_SORTER(h, 'h')
MAKE_ALPHABET_SORTER(i, 'i')
MAKE_ALPHABET_SORTER(j, 'j')
MAKE_ALPHABET_SORTER(k, 'k')
MAKE_ALPHABET_SORTER(l, 'l')
MAKE_ALPHABET_SORTER(m, 'm')
MAKE_ALPHABET_SORTER(n, 'n')
MAKE_ALPHABET_SORTER(o, 'o')
MAKE_ALPHABET_SORTER(p, 'p')
MAKE_ALPHABET_SORTER(q, 'q')
MAKE_ALPHABET_SORTER(r, 'r')
MAKE_ALPHABET_SORTER(s, 's')
MAKE_ALPHABET_SORTER(t, 't')
MAKE_ALPHABET_SORTER(u, 'u')
MAKE_ALPHABET_SORTER(v, 'v')
MAKE_ALPHABET_SORTER(w, 'w')
MAKE_ALPHABET_SORTER(x, 'x')
MAKE_ALPHABET_SORTER(y, 'y')
MAKE_ALPHABET_SORTER(z, 'z')

const std::vector<category_hierarchy_t> extra_category_hierarchy = {
    {SORT_CUSTOM_ULTIMATE,    SORT_ROOT,            NULL,                      "SMM_T0950_JA.png", NULL, NULL},
    {SORT_CUSTOM_OMNIMIX,     SORT_CUSTOM_ULTIMATE, custom_filter_omnimix,     "SMM_T0960_JA.png", NULL, NULL},
    {SORT_CUSTOM_JUBEAT_PLUS, SORT_CUSTOM_ULTIMATE, custom_filter_jubeat_plus, "SMM_T0961_JA.png", NULL, NULL},
    {SORT_CUSTOM_JUBEAT_2020, SORT_CUSTOM_ULTIMATE, custom_filter_jubeat_2020, "SMM_T0962_JA.png", NULL, NULL},
    {SORT_CUSTOM_JUKEBEAT,    SORT_CUSTOM_ULTIMATE, custom_filter_jukebeat,    "SMM_T0963_JA.png", NULL, NULL},
    {SORT_CUSTOM_A,           SORT_CATEGORY_NAME,   custom_filter_a,           "SMM_T0971_JA.png", NULL, NULL},
    {SORT_CUSTOM_B,           SORT_CATEGORY_NAME,   custom_filter_b,           "SMM_T0972_JA.png", NULL, NULL},
    {SORT_CUSTOM_C,           SORT_CATEGORY_NAME,   custom_filter_c,           "SMM_T0973_JA.png", NULL, NULL},
    {SORT_CUSTOM_D,           SORT_CATEGORY_NAME,   custom_filter_d,           "SMM_T0974_JA.png", NULL, NULL},
    {SORT_CUSTOM_E,           SORT_CATEGORY_NAME,   custom_filter_e,           "SMM_T0975_JA.png", NULL, NULL},
    {SORT_CUSTOM_F,           SORT_CATEGORY_NAME,   custom_filter_f,           "SMM_T0976_JA.png", NULL, NULL},
    {SORT_CUSTOM_G,           SORT_CATEGORY_NAME,   custom_filter_g,           "SMM_T0977_JA.png", NULL, NULL},
    {SORT_CUSTOM_H,           SORT_CATEGORY_NAME,   custom_filter_h,           "SMM_T0978_JA.png", NULL, NULL},
    {SORT_CUSTOM_I,           SORT_CATEGORY_NAME,   custom_filter_i,           "SMM_T0979_JA.png", NULL, NULL},
    {SORT_CUSTOM_J,           SORT_CATEGORY_NAME,   custom_filter_j,           "SMM_T0980_JA.png", NULL, NULL},
    {SORT_CUSTOM_K,           SORT_CATEGORY_NAME,   custom_filter_k,           "SMM_T0981_JA.png", NULL, NULL},
    {SORT_CUSTOM_L,           SORT_CATEGORY_NAME,   custom_filter_l,           "SMM_T0982_JA.png", NULL, NULL},
    {SORT_CUSTOM_M,           SORT_CATEGORY_NAME,   custom_filter_m,           "SMM_T0983_JA.png", NULL, NULL},
    {SORT_CUSTOM_N,           SORT_CATEGORY_NAME,   custom_filter_n,           "SMM_T0984_JA.png", NULL, NULL},
    {SORT_CUSTOM_O,           SORT_CATEGORY_NAME,   custom_filter_o,           "SMM_T0985_JA.png", NULL, NULL},
    {SORT_CUSTOM_P,           SORT_CATEGORY_NAME,   custom_filter_p,           "SMM_T0986_JA.png", NULL, NULL},
    {SORT_CUSTOM_Q,           SORT_CATEGORY_NAME,   custom_filter_q,           "SMM_T0987_JA.png", NULL, NULL},
    {SORT_CUSTOM_R,           SORT_CATEGORY_NAME,   custom_filter_r,           "SMM_T0988_JA.png", NULL, NULL},
    {SORT_CUSTOM_S,           SORT_CATEGORY_NAME,   custom_filter_s,           "SMM_T0989_JA.png", NULL, NULL},
    {SORT_CUSTOM_T,           SORT_CATEGORY_NAME,   custom_filter_t,           "SMM_T0990_JA.png", NULL, NULL},
    {SORT_CUSTOM_U,           SORT_CATEGORY_NAME,   custom_filter_u,           "SMM_T0991_JA.png", NULL, NULL},
    {SORT_CUSTOM_V,           SORT_CATEGORY_NAME,   custom_filter_v,           "SMM_T0992_JA.png", NULL, NULL},
    {SORT_CUSTOM_W,           SORT_CATEGORY_NAME,   custom_filter_w,           "SMM_T0993_JA.png", NULL, NULL},
    {SORT_CUSTOM_X,           SORT_CATEGORY_NAME,   custom_filter_x,           "SMM_T0994_JA.png", NULL, NULL},
    {SORT_CUSTOM_Y,           SORT_CATEGORY_NAME,   custom_filter_y,           "SMM_T0995_JA.png", NULL, NULL},
    {SORT_CUSTOM_Z,           SORT_CATEGORY_NAME,   custom_filter_z,           "SMM_T0996_JA.png", NULL, NULL},
};

const std::vector<category_listing_t> extra_category_layout = {
    // totally new folder
    {SORT_CUSTOM_ULTIMATE, 3, {
        {}, // leave first column blank
        {SORT_CUSTOM_OMNIMIX, SORT_CUSTOM_JUBEAT_PLUS, SORT_CUSTOM_JUBEAT_2020},
        {SORT_CUSTOM_JUKEBEAT},
    }},
    // Ultimate invalidates a lot of existing categories and adds our own.
    // Custom categories are prepended to the list, so we can totally replace
    // them by fully specifying it here
    {SORT_ROOT, 4, {
        // SORT_ROOT in SORT_ROOT is the "Select a category" icon
        {SORT_ROOT, SORT_CUSTOM_ULTIMATE, SORT_ALL},
        {SORT_CATEGORY_VERSION, SORT_CATEGORY_GENRE, SORT_CATEGORY_LEVEL},
        {SORT_CATEGORY_NAME, SORT_CATEGORY_ARTIST, SORT_HOLD},
        {SORT_TUNE_RUN, SORT_THIS_WEEK_RECOMMENDED, SORT_TARGET_MUSIC},
    }},
    {SORT_CATEGORY_NAME, 13, {
        {SORT_NULL, SORT_CUSTOM_A, SORT_CUSTOM_B},
        {SORT_CUSTOM_C, SORT_CUSTOM_D, SORT_CUSTOM_E},
        {SORT_CUSTOM_F, SORT_CUSTOM_G, SORT_CUSTOM_H},
        {SORT_CUSTOM_I, SORT_CUSTOM_J, SORT_CUSTOM_K},
        {SORT_CUSTOM_L, SORT_CUSTOM_M, SORT_CUSTOM_N},
        {SORT_CUSTOM_O, SORT_CUSTOM_P, SORT_CUSTOM_Q},
        {SORT_CUSTOM_R, SORT_CUSTOM_S, SORT_CUSTOM_T},
        {SORT_CUSTOM_U, SORT_CUSTOM_V, SORT_CUSTOM_W},
        {SORT_CUSTOM_X, SORT_CUSTOM_Y, SORT_CUSTOM_Z},
        // this extra NULL lets you fit every letter on one screen
        {SORT_NULL, SORT_JP_A, SORT_JP_KA},
        {SORT_JP_SA, SORT_JP_TA, SORT_JP_NA},
        {SORT_JP_HA, SORT_JP_MA, SORT_JP_YA},
        {SORT_JP_RA, SORT_JP_WA},
    }}
};

enum custom_group_id: uint32_t {
    GROUP_CUSTOM_0_9 = GROUP_DEFAULT_MAX_ID,
    GROUP_CUSTOM_A,
    GROUP_CUSTOM_B,
    GROUP_CUSTOM_C,
    GROUP_CUSTOM_D,
    GROUP_CUSTOM_E,
    GROUP_CUSTOM_F,
    GROUP_CUSTOM_G,
    GROUP_CUSTOM_H,
    GROUP_CUSTOM_I,
    GROUP_CUSTOM_J,
    GROUP_CUSTOM_K,
    GROUP_CUSTOM_L,
    GROUP_CUSTOM_M,
    GROUP_CUSTOM_N,
    GROUP_CUSTOM_O,
    GROUP_CUSTOM_P,
    GROUP_CUSTOM_Q,
    GROUP_CUSTOM_R,
    GROUP_CUSTOM_S,
    GROUP_CUSTOM_T,
    GROUP_CUSTOM_U,
    GROUP_CUSTOM_V,
    GROUP_CUSTOM_W,
    GROUP_CUSTOM_X,
    GROUP_CUSTOM_Y,
    GROUP_CUSTOM_Z,
};

// Grouping in song select
const std::vector<grouping_textures_t> extra_group_textures = {
    {GROUP_CUSTOM_0_9, "SMM_T2710_JA.png", NULL, NULL},
    {GROUP_CUSTOM_A, "SMM_T2711_JA.png", NULL, NULL},
    {GROUP_CUSTOM_B, "SMM_T2712_JA.png", NULL, NULL},
    {GROUP_CUSTOM_C, "SMM_T2713_JA.png", NULL, NULL},
    {GROUP_CUSTOM_D, "SMM_T2714_JA.png", NULL, NULL},
    {GROUP_CUSTOM_E, "SMM_T2715_JA.png", NULL, NULL},
    {GROUP_CUSTOM_F, "SMM_T2716_JA.png", NULL, NULL},
    {GROUP_CUSTOM_G, "SMM_T2717_JA.png", NULL, NULL},
    {GROUP_CUSTOM_H, "SMM_T2718_JA.png", NULL, NULL},
    {GROUP_CUSTOM_I, "SMM_T2719_JA.png", NULL, NULL},
    {GROUP_CUSTOM_J, "SMM_T2720_JA.png", NULL, NULL},
    {GROUP_CUSTOM_K, "SMM_T2721_JA.png", NULL, NULL},
    {GROUP_CUSTOM_L, "SMM_T2722_JA.png", NULL, NULL},
    {GROUP_CUSTOM_M, "SMM_T2723_JA.png", NULL, NULL},
    {GROUP_CUSTOM_N, "SMM_T2724_JA.png", NULL, NULL},
    {GROUP_CUSTOM_O, "SMM_T2725_JA.png", NULL, NULL},
    {GROUP_CUSTOM_P, "SMM_T2726_JA.png", NULL, NULL},
    {GROUP_CUSTOM_Q, "SMM_T2727_JA.png", NULL, NULL},
    {GROUP_CUSTOM_R, "SMM_T2728_JA.png", NULL, NULL},
    {GROUP_CUSTOM_S, "SMM_T2729_JA.png", NULL, NULL},
    {GROUP_CUSTOM_T, "SMM_T2730_JA.png", NULL, NULL},
    {GROUP_CUSTOM_U, "SMM_T2731_JA.png", NULL, NULL},
    {GROUP_CUSTOM_V, "SMM_T2732_JA.png", NULL, NULL},
    {GROUP_CUSTOM_W, "SMM_T2733_JA.png", NULL, NULL},
    {GROUP_CUSTOM_X, "SMM_T2734_JA.png", NULL, NULL},
    {GROUP_CUSTOM_Y, "SMM_T2735_JA.png", NULL, NULL},
    {GROUP_CUSTOM_Z, "SMM_T2736_JA.png", NULL, NULL},
};

uint32_t __fastcall category_group_fn_alphabet(enum group_type group_type, const music_info_for_grouping_t *info) {
    if(group_type != GROUP_TYPE_NAME || !info) {
        return GROUP_INVALID;
    }
    auto song = music_from_id(info->id);
    auto first = song->sort_name[0];

    if(first >= '0' && first <= '9') {
        return GROUP_CUSTOM_0_9;
    }

    if(first >= 'a' && first <= 'z') {
        return GROUP_CUSTOM_A + (first - 'a');
    }

    return GROUP_INVALID;
}
