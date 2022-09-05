#include <stdint.h>
#include <vector>

/* How you might use this module:

enum custom_sort_id: uint32_t {
    SORT_CUSTOM_TEST_ROOT = SORT_DEFAULT_MAX_ID,
    SORT_CUSTOM_TEST_FOLDER1,
    SORT_CUSTOM_TEST_FOLDER2,
};

static bool __cdecl custom_test_child1_filter(unsigned music_id, int diff, uint8_t level) {
    // ghostbusters
    return music_id == 411190003;
}
static bool __cdecl custom_test_child2_filter(unsigned music_id, int diff, uint8_t level) {
    // africa
    return music_id == 411170003;
}

static const std::vector<category_hierarchy_t> extra_category_hierarchy = {
    {SORT_CUSTOM_TEST_ROOT, SORT_ROOT, NULL, "custom_sort_root.png"},
    {SORT_CUSTOM_TEST_FOLDER1, SORT_CUSTOM_TEST_ROOT, custom_test_child1_filter, "custom_sort_child1.png"},
    {SORT_CUSTOM_TEST_FOLDER2, SORT_CUSTOM_TEST_ROOT, custom_test_child2_filter, "custom_sort_child2.png"},
};

static const std::vector<category_listing_t> extra_category_layout = {
    {SORT_CUSTOM_TEST_ROOT, 2, {
        {SORT_NULL, SORT_CUSTOM_TEST_FOLDER1, SORT_CUSTOM_TEST_FOLDER2},
    }},
};

void init() {
    MH_Initialize();

    category_hooks_add_category_definitions(extra_category_hierarchy);
    category_hooks_add_category_layouts(extra_category_layout);
    category_hooks_init(process, jubeat_info);

    auto root = category_hooks_get_listing(SORT_ROOT);
    // add our extension category to the root folder, it has 7 columns by default
    root->column_count++;
    root->columns[7][0] = SORT_CUSTOM_TEST_ROOT;

    MH_EnableHook(MH_ALL_HOOKS);
}
*/

enum folder_sort_id: uint32_t {
    SORT_NULL = 0, // empty tile with nothing to click on
    SORT_ROOT = 1,
    SORT_ALL = 2,
    SORT_CATEGORY_GENRE_QB = 3, // special Qubell-only genre with different colour
    SORT_YOU_MIGHT_LIKE = 4,
    SORT_CATEGORY_GENRE = 5,
    SORT_POPS = 6,
    SORT_ANIME = 7,
    SORT_SOCIAL_MUSIC = 8,
    SORT_TOUHOU_ARRANGE = 9,
    SORT_GAME = 10,
    SORT_CLASSIC = 11,
    SORT_ORIGINAL = 12,
    SORT_CATEGORY_LEVEL = 13,
    SORT_LEVEL1 = 14,
    SORT_LEVEL2 = 15,
    SORT_LEVEL3 = 16,
    SORT_LEVEL4 = 17,
    SORT_LEVEL5 = 18,
    SORT_LEVEL6 = 19,
    SORT_LEVEL7 = 20,
    SORT_LEVEL8 = 21,
    SORT_LEVEL9 = 22,
    SORT_LEVEL10 = 23,
    SORT_CATEGORY_NAME = 24,
    SORT_JP_A = 25,
    SORT_JP_KA = 26,
    SORT_JP_SA = 27,
    SORT_JP_TA = 28,
    SORT_JP_NA = 29,
    SORT_JP_HA = 30,
    SORT_JP_MA = 31,
    SORT_JP_YA = 32,
    SORT_JP_RA = 33,
    SORT_JP_WA = 34,
    SORT_HOLD = 35,
    SORT_IS_NEW = 36,
    SORT_NATIONAL_POPULAR = 37,
    SORT_FULLCOMBO_CHALLENGE = 38,
    SORT_CATEGORY_PERFORMANCE = 39,
    SORT_UNPLAYED = 40,
    SORT_IS_FULL_COMBO1 = 41,
    SORT_IS_FULL_COMBO2 = 42,
    SORT_IS_NO_GRAY1 = 43,
    SORT_IS_NO_GRAY2 = 44,
    SORT_IS_ALL_YELLOW1 = 45,
    SORT_IS_ALL_YELLOW2 = 46,
    SORT_RANK_E = 47,
    SORT_RANK_D = 48,
    SORT_RANK_C = 49,
    SORT_RANK_B = 50,
    SORT_RANK_A = 51,
    SORT_RANK_S = 52,
    SORT_RANK_SS = 53,
    SORT_RANK_SSS = 54,
    SORT_RANK_EXC = 55,
    SORT_MYBEST = 56,
    SORT_CATEGORY_TOURNAMENT_PARTICIPATION = 57,
    SORT_TOURNAMENT_1 = 58,
    SORT_TOURNAMENT_2 = 59,
    SORT_TOURNAMENT_3 = 60,
    SORT_TOURNAMENT_4 = 61,
    SORT_TOURNAMENT_5 = 62,
    SORT_CATEGORY_JUBEAT_LAB = 63,
    SORT_LAB_MY_LIST = 64,
    SORT_LAB_DOWNLOAD_RANKING = 65,
    SORT_LAB_PLAYCOUNT_RANKING = 66,
    SORT_LAB_GOOD_CHART_RANKING = 67,
    SORT_IS_WEEKLY_TARGET = 68,
    SORT_CATEGORY_VERSION = 74,
    SORT_VER_jubeat = 75,
    SORT_VER_ripples = 76,
    SORT_VER_knit = 77,
    SORT_VER_copious = 78,
    SORT_VER_saucer = 79,
    SORT_VER_saucer_fulfill = 80,
    SORT_VER_prop = 81,
    SORT_VER_Qubell = 82,
    SORT_VER_clan = 83,
    SORT_VER_festo = 84,
    SORT_CATEGORY_ARTIST = 85,
    SORT_ARTIST_Sota_Fujimori = 86,
    SORT_ARTIST_DJ_YOSHITAKA = 87,
    SORT_ARTIST_Nekomata_Master = 88,
    SORT_ARTIST_TAG = 89,
    SORT_ARTIST_wac = 90,
    SORT_ARTIST_LED = 91,
    SORT_ARTIST_Hinabita = 92,
    SORT_ARTIST_Akhuta = 93,
    SORT_ARTIST_Tomoaki_Hirono = 94,
    SORT_ARTIST_S_C_U = 95,
    SORT_ARTIST_Ryu = 96,
    SORT_ARTIST_kors_k = 97,
    SORT_ARTIST_dj_TAKA = 98,
    SORT_ARTIST_DJ_TOTTO = 99,
    SORT_ARTIST_Mutsuhiko_Izumi = 100,
    SORT_ARTIST_Yoshihiko_Koezuka = 101,
    SORT_ARTIST_TOMOSUKE = 102,
    SORT_ARTIST_Asaki = 103,
    SORT_ARTIST_seiya_murai = 104,
    SORT_ARTIST_PON = 105,
    SORT_ARTIST_Qrispy_Joybox = 106,
    SORT_ARTIST_GUHROOVY = 107,
    SORT_ARTIST_U1_ASAMi = 108,
    SORT_ARTIST_Hommarju = 109,
    SORT_ARTIST_ARM = 110,
    SORT_ARTIST_AOP = 111,
    SORT_ARTIST_NEKOHIROKI = 112,
    SORT_ARTIST_Mayumi_Morinaga = 113,
    SORT_SHOP = 114,
    SORT_PJ_MATCH = 115,
    SORT_TUNE_RUN = 118,
    SORT_TARGET_MUSIC = 121,
    SORT_THIS_WEEK_RECOMMENDED = 122,

    SORT_DEFAULT_MAX_ID,
};

typedef bool (__cdecl *music_sort_function)(unsigned music_id, int diff, uint8_t level);

typedef struct {
    // not enum folder_sort_id as the extra casts for custom values suck
    uint32_t id;
    uint32_t parent;
    music_sort_function sort_fn; // NULL for non-leaf nodes
    const char *tex_name_lang_j;
    const char *tex_name_lang_k; // can be NULL, J will be used
    const char *tex_name_lang_a; // can be NULL, J will be used
} category_hierarchy_t;

typedef struct {
    uint32_t id;
    int column_count;
    uint32_t columns[16][3];
} category_listing_t;

// must match game's expectation
static_assert(sizeof(category_listing_t) == 200);

// must be called before category_hooks_init
void category_hooks_add_category_definitions(std::vector<category_hierarchy_t> categories);
void category_hooks_add_category_layouts(std::vector<category_listing_t> listings);

void category_hooks_init(HANDLE process, const MODULEINFO &jubeat_info);

// must be called after category_hooks_init
// used to add your custom categories to the root folder
category_listing_t *category_hooks_get_listing(uint32_t id);
