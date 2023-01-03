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

    // T9
    SORT_CUSTOM_ABC,
    SORT_CUSTOM_DEF,
    SORT_CUSTOM_GHI,
    SORT_CUSTOM_JKL,
    SORT_CUSTOM_MNO,
    SORT_CUSTOM_PQRS,
    SORT_CUSTOM_TUV,
    SORT_CUSTOM_WXYZ,

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
    {SORT_CUSTOM_ULTIMATE,    SORT_ROOT,            NULL,                      "UL_ROOT.png", NULL, NULL},
    {SORT_CUSTOM_OMNIMIX,     SORT_CUSTOM_ULTIMATE, custom_filter_omnimix,     "UL_SORT_OMNI.png", NULL, NULL},
    {SORT_CUSTOM_JUBEAT_PLUS, SORT_CUSTOM_ULTIMATE, custom_filter_jubeat_plus, "UL_SORT_JBPLUS.png", NULL, NULL},
    {SORT_CUSTOM_JUBEAT_2020, SORT_CUSTOM_ULTIMATE, custom_filter_jubeat_2020, "UL_SORT_JB2020.png", NULL, NULL},
    {SORT_CUSTOM_JUKEBEAT,    SORT_CUSTOM_ULTIMATE, custom_filter_jukebeat,    "UL_SORT_JUKEBEAT.png", NULL, NULL},
    {SORT_CUSTOM_ABC,         SORT_CATEGORY_NAME,   NULL,                      "UL_SORT_ABC.png", NULL, NULL},
    {SORT_CUSTOM_DEF,         SORT_CATEGORY_NAME,   NULL,                      "UL_SORT_DEF.png", NULL, NULL},
    {SORT_CUSTOM_GHI,         SORT_CATEGORY_NAME,   NULL,                      "UL_SORT_GHI.png", NULL, NULL},
    {SORT_CUSTOM_JKL,         SORT_CATEGORY_NAME,   NULL,                      "UL_SORT_JKL.png", NULL, NULL},
    {SORT_CUSTOM_MNO,         SORT_CATEGORY_NAME,   NULL,                      "UL_SORT_MNO.png", NULL, NULL},
    {SORT_CUSTOM_PQRS,        SORT_CATEGORY_NAME,   NULL,                      "UL_SORT_PQRS.png", NULL, NULL},
    {SORT_CUSTOM_TUV,         SORT_CATEGORY_NAME,   NULL,                      "UL_SORT_TUV.png", NULL, NULL},
    {SORT_CUSTOM_WXYZ,        SORT_CATEGORY_NAME,   NULL,                      "UL_SORT_WXYZ.png", NULL, NULL},
    {SORT_CUSTOM_A,           SORT_CUSTOM_ABC,      custom_filter_a,           "UL_SORT_A.png", NULL, NULL},
    {SORT_CUSTOM_B,           SORT_CUSTOM_ABC,      custom_filter_b,           "UL_SORT_B.png", NULL, NULL},
    {SORT_CUSTOM_C,           SORT_CUSTOM_ABC,      custom_filter_c,           "UL_SORT_C.png", NULL, NULL},
    {SORT_CUSTOM_D,           SORT_CUSTOM_DEF,      custom_filter_d,           "UL_SORT_D.png", NULL, NULL},
    {SORT_CUSTOM_E,           SORT_CUSTOM_DEF,      custom_filter_e,           "UL_SORT_E.png", NULL, NULL},
    {SORT_CUSTOM_F,           SORT_CUSTOM_DEF,      custom_filter_f,           "UL_SORT_F.png", NULL, NULL},
    {SORT_CUSTOM_G,           SORT_CUSTOM_GHI,      custom_filter_g,           "UL_SORT_G.png", NULL, NULL},
    {SORT_CUSTOM_H,           SORT_CUSTOM_GHI,      custom_filter_h,           "UL_SORT_H.png", NULL, NULL},
    {SORT_CUSTOM_I,           SORT_CUSTOM_GHI,      custom_filter_i,           "UL_SORT_I.png", NULL, NULL},
    {SORT_CUSTOM_J,           SORT_CUSTOM_JKL,      custom_filter_j,           "UL_SORT_J.png", NULL, NULL},
    {SORT_CUSTOM_K,           SORT_CUSTOM_JKL,      custom_filter_k,           "UL_SORT_K.png", NULL, NULL},
    {SORT_CUSTOM_L,           SORT_CUSTOM_JKL,      custom_filter_l,           "UL_SORT_L.png", NULL, NULL},
    {SORT_CUSTOM_M,           SORT_CUSTOM_MNO,      custom_filter_m,           "UL_SORT_M.png", NULL, NULL},
    {SORT_CUSTOM_N,           SORT_CUSTOM_MNO,      custom_filter_n,           "UL_SORT_N.png", NULL, NULL},
    {SORT_CUSTOM_O,           SORT_CUSTOM_MNO,      custom_filter_o,           "UL_SORT_O.png", NULL, NULL},
    {SORT_CUSTOM_P,           SORT_CUSTOM_PQRS,     custom_filter_p,           "UL_SORT_P.png", NULL, NULL},
    {SORT_CUSTOM_Q,           SORT_CUSTOM_PQRS,     custom_filter_q,           "UL_SORT_Q.png", NULL, NULL},
    {SORT_CUSTOM_R,           SORT_CUSTOM_PQRS,     custom_filter_r,           "UL_SORT_R.png", NULL, NULL},
    {SORT_CUSTOM_S,           SORT_CUSTOM_PQRS,     custom_filter_s,           "UL_SORT_S.png", NULL, NULL},
    {SORT_CUSTOM_T,           SORT_CUSTOM_TUV,      custom_filter_t,           "UL_SORT_T.png", NULL, NULL},
    {SORT_CUSTOM_U,           SORT_CUSTOM_TUV,      custom_filter_u,           "UL_SORT_U.png", NULL, NULL},
    {SORT_CUSTOM_V,           SORT_CUSTOM_TUV,      custom_filter_v,           "UL_SORT_V.png", NULL, NULL},
    {SORT_CUSTOM_W,           SORT_CUSTOM_WXYZ,     custom_filter_w,           "UL_SORT_W.png", NULL, NULL},
    {SORT_CUSTOM_X,           SORT_CUSTOM_WXYZ,     custom_filter_x,           "UL_SORT_X.png", NULL, NULL},
    {SORT_CUSTOM_Y,           SORT_CUSTOM_WXYZ,     custom_filter_y,           "UL_SORT_Y.png", NULL, NULL},
    {SORT_CUSTOM_Z,           SORT_CUSTOM_WXYZ,     custom_filter_z,           "UL_SORT_Z.png", NULL, NULL},
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
    {SORT_CATEGORY_NAME, 7, {
        {SORT_NULL, SORT_CUSTOM_GHI, SORT_CUSTOM_PQRS},
        {SORT_CUSTOM_ABC, SORT_CUSTOM_JKL, SORT_CUSTOM_TUV},
        {SORT_CUSTOM_DEF, SORT_CUSTOM_MNO, SORT_CUSTOM_WXYZ},
        // this extra NULL lets you fit every letter on one screen
        {SORT_NULL, SORT_JP_A, SORT_JP_KA},
        {SORT_JP_SA, SORT_JP_TA, SORT_JP_NA},
        {SORT_JP_HA, SORT_JP_MA, SORT_JP_YA},
        {SORT_JP_RA, SORT_JP_WA},
    }},

    // Each individual subfolder for the T9 keyboard is a clone of the original
    // name sort, just with the letters you selected expanded. Also, the
    // expanded letters do not overlap the button you pressed initially, which
    // instead doubles as a "back" button. These two design choices make it more
    // ergonomic to quickly move around if you change your mind.
    // The extra buttons appear below, preferably, or above the selected button.
    {SORT_CUSTOM_ABC, 7, {
        {SORT_NULL, SORT_CUSTOM_A, SORT_CUSTOM_PQRS},
        {SORT_CATEGORY_NAME, SORT_CUSTOM_B, SORT_CUSTOM_TUV},
        {SORT_CUSTOM_DEF, SORT_CUSTOM_C, SORT_CUSTOM_WXYZ},
        {SORT_NULL, SORT_JP_A, SORT_JP_KA},
        {SORT_JP_SA, SORT_JP_TA, SORT_JP_NA},
        {SORT_JP_HA, SORT_JP_MA, SORT_JP_YA},
        {SORT_JP_RA, SORT_JP_WA},
    }},
    {SORT_CUSTOM_DEF, 7, {
        {SORT_NULL, SORT_CUSTOM_D, SORT_CUSTOM_PQRS},
        {SORT_CUSTOM_ABC, SORT_CUSTOM_E, SORT_CUSTOM_TUV},
        {SORT_CATEGORY_NAME, SORT_CUSTOM_F, SORT_CUSTOM_WXYZ},
        {SORT_NULL, SORT_JP_A, SORT_JP_KA},
        {SORT_JP_SA, SORT_JP_TA, SORT_JP_NA},
        {SORT_JP_HA, SORT_JP_MA, SORT_JP_YA},
        {SORT_JP_RA, SORT_JP_WA},
    }},
    {SORT_CUSTOM_GHI, 7, {
        {SORT_NULL, SORT_CATEGORY_NAME, SORT_CUSTOM_G},
        {SORT_CUSTOM_ABC, SORT_CUSTOM_JKL, SORT_CUSTOM_H},
        {SORT_CUSTOM_DEF, SORT_CUSTOM_MNO, SORT_CUSTOM_I},
        {SORT_NULL, SORT_JP_A, SORT_JP_KA},
        {SORT_JP_SA, SORT_JP_TA, SORT_JP_NA},
        {SORT_JP_HA, SORT_JP_MA, SORT_JP_YA},
        {SORT_JP_RA, SORT_JP_WA},
    }},
    {SORT_CUSTOM_JKL, 7, {
        {SORT_NULL, SORT_CUSTOM_GHI, SORT_CUSTOM_J},
        {SORT_CUSTOM_ABC, SORT_CATEGORY_NAME, SORT_CUSTOM_K},
        {SORT_CUSTOM_DEF, SORT_CUSTOM_MNO, SORT_CUSTOM_L},
        {SORT_NULL, SORT_JP_A, SORT_JP_KA},
        {SORT_JP_SA, SORT_JP_TA, SORT_JP_NA},
        {SORT_JP_HA, SORT_JP_MA, SORT_JP_YA},
        {SORT_JP_RA, SORT_JP_WA},
    }},
    {SORT_CUSTOM_MNO, 7, {
        {SORT_NULL, SORT_CUSTOM_GHI, SORT_CUSTOM_M},
        {SORT_CUSTOM_ABC, SORT_CUSTOM_JKL, SORT_CUSTOM_N},
        {SORT_CUSTOM_DEF, SORT_CATEGORY_NAME, SORT_CUSTOM_O},
        {SORT_NULL, SORT_JP_A, SORT_JP_KA},
        {SORT_JP_SA, SORT_JP_TA, SORT_JP_NA},
        {SORT_JP_HA, SORT_JP_MA, SORT_JP_YA},
        {SORT_JP_RA, SORT_JP_WA},
    }},
    {SORT_CUSTOM_PQRS, 7, {
        {SORT_NULL, SORT_CUSTOM_P, SORT_CATEGORY_NAME},
        {SORT_CUSTOM_ABC, SORT_CUSTOM_Q, SORT_CUSTOM_TUV},
        {SORT_CUSTOM_DEF, SORT_CUSTOM_R, SORT_CUSTOM_WXYZ},
        {SORT_NULL, SORT_CUSTOM_S, SORT_JP_KA},
        {SORT_JP_SA, SORT_JP_TA, SORT_JP_NA},
        {SORT_JP_HA, SORT_JP_MA, SORT_JP_YA},
        {SORT_JP_RA, SORT_JP_WA},
    }},
    {SORT_CUSTOM_TUV, 7, {
        {SORT_NULL, SORT_CUSTOM_T, SORT_CUSTOM_PQRS},
        {SORT_CUSTOM_ABC, SORT_CUSTOM_U, SORT_CATEGORY_NAME},
        {SORT_CUSTOM_DEF, SORT_CUSTOM_V, SORT_CUSTOM_WXYZ},
        {SORT_NULL, SORT_JP_A, SORT_JP_KA},
        {SORT_JP_SA, SORT_JP_TA, SORT_JP_NA},
        {SORT_JP_HA, SORT_JP_MA, SORT_JP_YA},
        {SORT_JP_RA, SORT_JP_WA},
    }},
    {SORT_CUSTOM_WXYZ, 7, {
        {SORT_NULL, SORT_CUSTOM_W, SORT_CUSTOM_PQRS},
        {SORT_CUSTOM_ABC, SORT_CUSTOM_X, SORT_CUSTOM_TUV},
        {SORT_CUSTOM_DEF, SORT_CUSTOM_Y, SORT_CATEGORY_NAME},
        {SORT_NULL, SORT_CUSTOM_Z, SORT_JP_KA},
        {SORT_JP_SA, SORT_JP_TA, SORT_JP_NA},
        {SORT_JP_HA, SORT_JP_MA, SORT_JP_YA},
        {SORT_JP_RA, SORT_JP_WA},
    }},
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
    {GROUP_CUSTOM_0_9, "UL_GROUP_0_9.png", NULL, NULL},
    {GROUP_CUSTOM_A, "UL_GROUP_A.png", NULL, NULL},
    {GROUP_CUSTOM_B, "UL_GROUP_B.png", NULL, NULL},
    {GROUP_CUSTOM_C, "UL_GROUP_C.png", NULL, NULL},
    {GROUP_CUSTOM_D, "UL_GROUP_D.png", NULL, NULL},
    {GROUP_CUSTOM_E, "UL_GROUP_E.png", NULL, NULL},
    {GROUP_CUSTOM_F, "UL_GROUP_F.png", NULL, NULL},
    {GROUP_CUSTOM_G, "UL_GROUP_G.png", NULL, NULL},
    {GROUP_CUSTOM_H, "UL_GROUP_H.png", NULL, NULL},
    {GROUP_CUSTOM_I, "UL_GROUP_I.png", NULL, NULL},
    {GROUP_CUSTOM_J, "UL_GROUP_J.png", NULL, NULL},
    {GROUP_CUSTOM_K, "UL_GROUP_K.png", NULL, NULL},
    {GROUP_CUSTOM_L, "UL_GROUP_L.png", NULL, NULL},
    {GROUP_CUSTOM_M, "UL_GROUP_M.png", NULL, NULL},
    {GROUP_CUSTOM_N, "UL_GROUP_N.png", NULL, NULL},
    {GROUP_CUSTOM_O, "UL_GROUP_O.png", NULL, NULL},
    {GROUP_CUSTOM_P, "UL_GROUP_P.png", NULL, NULL},
    {GROUP_CUSTOM_Q, "UL_GROUP_Q.png", NULL, NULL},
    {GROUP_CUSTOM_R, "UL_GROUP_R.png", NULL, NULL},
    {GROUP_CUSTOM_S, "UL_GROUP_S.png", NULL, NULL},
    {GROUP_CUSTOM_T, "UL_GROUP_T.png", NULL, NULL},
    {GROUP_CUSTOM_U, "UL_GROUP_U.png", NULL, NULL},
    {GROUP_CUSTOM_V, "UL_GROUP_V.png", NULL, NULL},
    {GROUP_CUSTOM_W, "UL_GROUP_W.png", NULL, NULL},
    {GROUP_CUSTOM_X, "UL_GROUP_X.png", NULL, NULL},
    {GROUP_CUSTOM_Y, "UL_GROUP_Y.png", NULL, NULL},
    {GROUP_CUSTOM_Z, "UL_GROUP_Z.png", NULL, NULL},
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
