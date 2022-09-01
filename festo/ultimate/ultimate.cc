#define LOG_MODULE "ultimate"

#include <vector>

// clang-format off
#include <windows.h>
#include <psapi.h>
// clang-format on

#include <stdint.h>

#include "hook/table.h"

#include "imports/avs2-core/avs.h"
#include "imports/gftools.h"
#include "imports/jubeat.h"

#include "pattern/pattern.h"

#include "pe/iat.h"

#include "util/defs.h"
#include "util/log.h"
#include "util/mem.h"
#include "util/x86.h"

#include "categories.h"
#include "music_db.h"
#include "pkfs.h"

#include "../common/bnr_hook.h"
#include "../common/festo.h"

#include "MinHook.h"

// clang-format off

const struct patch_t packlist_pluslist {
    .name = "ultilist patch",
    .pattern = { 'p', 'a', 'c', 'k', 'l', 'i', 's', 't' },
    .data = { 'u', 'l', 't', 'i' },
    .data_offset = 0,
};

const struct patch_t smc_mm_text_ja {
    .name = "smc_mm_text_ja patch",
    .pattern = { 'S', 'M', 'C', '_', 'M', 'M', '_', 'T', 'E', 'X', 'T', '_', 'J', 'A' },
    .data = { 'E', 'X' },
    .data_offset = 12,
};

const struct patch_t smc_mm_text_ko {
    .name = "smc_mm_text_ko patch",
    .pattern = { 'S', 'M', 'C', '_', 'M', 'M', '_', 'T', 'E', 'X', 'T', '_', 'K', 'O' },
    .data = { 'E', 'X' },
    .data_offset = 12,
};

const struct patch_t smc_mm_hierarchy_ja {
    .name = "smc_mm_hierarchy_ja patch",
    .pattern = { 'S', 'M', 'C', '_', 'M', 'M', '_', 'H', 'I', 'E', 'R', 'A', 'R', 'C', 'H', 'Y', '_', 'J', 'A' },
    .data = { 'E', 'X' },
    .data_offset = 17,
};

const struct patch_t smc_mm_hierarchy_ko {
    .name = "smc_mm_hierarchy_ko patch",
    .pattern = { 'S', 'M', 'C', '_', 'M', 'M', '_', 'H', 'I', 'E', 'R', 'A', 'R', 'C', 'H', 'Y', '_', 'K', 'O' },
    .data = { 'E', 'X' },
    .data_offset = 17,
};

// clang-format on

static int stack_replacer[MAX_SONGS];
static const uintptr_t mdb_arr_patch = reinterpret_cast<uintptr_t>(stack_replacer);

static uint8_t score_stack_replacer[MAX_SONGS][20];
static const uintptr_t score_arr_patch = reinterpret_cast<uintptr_t>(score_stack_replacer);
// the asm to replace needs the end of the array
static const uintptr_t end_score_arr_patch =
    reinterpret_cast<uintptr_t>(&score_stack_replacer[MAX_SONGS]);

// clang-format off

const struct patch_t mdb_array_1_0 {
    .name = "mdb 1.0",
    .pattern = { 0x00, 0x68, 0x00, 0x08, 0x00, 0x00, 0x50 },
    .data = { U32_TO_CONST_BYTES_LE(MAX_SONGS) },
    .data_offset = 2,
};
const struct patch_t mdb_array_1_1 {
    .name = "mdb 1.1",
    .pattern = { 0x8D, 0x84, 0x24, 0x38, 0x02, 0x00, 0x00 },
    .data = { 0xB8, U32_TO_CONST_BYTES_LE(mdb_arr_patch), 0x90, 0x90 },
    .data_offset = 0,
};
const struct patch_t mdb_array_1_2 {
    .name = "mdb 1.2",
    .pattern = { 0x8B, 0x8C, 0x84, 0x38, 0x02, 0x00, 0x00 },
    .data = { 0x8B, 0x0C, 0x85, U32_TO_CONST_BYTES_LE(mdb_arr_patch) },
    .data_offset = 0,
};
const struct patch_t mdb_array_2_0 {
    .name = "mdb 2.0",
    .pattern = { 0x68, 0x00, 0x08, 0x00, 0x00, 0x8D, 0x8C, 0x24, 0x3C },
    .data = { U32_TO_CONST_BYTES_LE(MAX_SONGS) },
    .data_offset = 1,
};
const struct patch_t mdb_array_2_1 {
    .name = "mdb 2.1",
    .pattern = { 0x8D, 0x8C, 0x24, 0x3C, 0x02, 0x00, 0x00 },
    .data = { 0xB9, U32_TO_CONST_BYTES_LE(mdb_arr_patch), 0x90, 0x90 },
    .data_offset = 0,
};
const struct patch_t mdb_array_2_2 {
    .name = "mdb 2.2",
    .pattern = { 0x8B, 0x94, 0x9C, 0x38, 0x02, 0x00, 0x00 },
    .data = { 0x8B, 0x14, 0x9D, U32_TO_CONST_BYTES_LE(mdb_arr_patch) },
    .data_offset = 0,
};

// list passed to music_db_get_all_permitted_list
const struct patch_t mdb_array_3_0 {
    .name = "mdb 3.0",
    .pattern = { 0x56, 0x57, 0x8D, 0x85, 0xFC, 0xDF, 0xFF, 0xFF },
    .data = {
        0xB8, // mov eax, imm32
        U32_TO_CONST_BYTES_LE(mdb_arr_patch),
        0x90,
    },
    .data_offset = 2,
};

// size passed to music_db_get_all_permitted_list
const struct patch_t mdb_array_3_1 {
    .name = "mdb 3.1",
    .pattern = { 0x8B, 0xD9, 0x50, 0x68, 0x00, 0x08, 0x00, 0x00 },
    .data = { U32_TO_CONST_BYTES_LE(MAX_SONGS) },
    .data_offset = 4,
};

// iteration list deref
const struct patch_t mdb_array_3_2 {
    .name = "mdb 3.2",
    .pattern = { 0x8B, 0xB4, 0x85, 0xFC, 0xDF, 0xFF, 0xFF }, // mov esi, [ebp+eax*4+permitted_list]
    .data = {
        0x8B,
        0x34,
        0x85, // mov esi, [eax*4+imm32]
        U32_TO_CONST_BYTES_LE(mdb_arr_patch),
    },
    .data_offset = 0,
};

// score info ref
const struct patch_t mdb_array_3_3 {
    .name = "mdb 3.3",
    .pattern = { 0x8D, 0x8D, 0xFC, 0xFF, 0xFD, 0xFF },
    .data = {
        0xB9, // mov ecx, imm32
        U32_TO_CONST_BYTES_LE(score_arr_patch),
        0x90,
    },
    .data_offset = 0,
};

// score info ref #2, passed to avs_qsort
const struct patch_t mdb_array_3_4 {
    .name = "mdb 3.4",
    .pattern = { 0x8D, 0x85, 0xFC, 0xFF, 0xFD, 0xFF },
    .data = {
        0xB8, // mov eax, imm32
        U32_TO_CONST_BYTES_LE(score_arr_patch),
        0x90,
    },
    .data_offset = 0,
};

// score info ref #3
const struct patch_t mdb_array_3_5 {
    .name = "mdb 3.5",
    .pattern = { 0x8D, 0xB5, 0xFC, 0xFF, 0xFD, 0xFF }, // lea esi, [ebp+score_info]
    .data = {
        0xBE, // mov esi, imm32
        U32_TO_CONST_BYTES_LE(score_arr_patch),
        0x90,
    },
    .data_offset = 0,
};

// loop terminator, end of score array
const struct patch_t mdb_array_3_6 {
    .name = "mdb 3.6",
    .pattern = { 0xC6, 0x14, 0x8D, 0x85, 0xFC, 0xDF, 0xFF, 0xFF },
    .data = {
        0xB8, // mov eax, imm32
        U32_TO_CONST_BYTES_LE(end_score_arr_patch),
        0x90,
    },
    .data_offset = 2,
};

// omnimix and extend song textures
static std::vector<const char *> BNR_TEXTURES {
    "L44FO_BNR_J_OM_001",
    "L44FO_BNR_J_OM_002",
    "L44FO_BNR_J_EX_001",
    "L44FO_BNR_J_EX_002",
    "L44FO_BNR_J_EX_003",
    "L44FO_BNR_J_EX_004",
};

// markers and backgrounds
static std::vector<const char *> EXTRA_MARKERS {
    "L44_TM_BANNER_OM",
};
static std::vector<const char *> EXTRA_BACKGROUNDS {
    "L44FO_PLAY_BACKGROUND_OM",
    // "L44FO_PLAY_BACKGROUND_UL_01",
    // "L44FO_PLAY_BACKGROUND_UL_02",
    // "L44FO_PLAY_BACKGROUND_UL_03",
};
static std::vector<const char *> EXTRA_BG_CHANGES {
    "L44FO_STG_BG_CHANGE_OM",
    // "L44FO_STG_BG_CHANGE_UL_01",
    // "L44FO_STG_BG_CHANGE_UL_02",
};

// ultimate categories/folders
static std::vector<const char *> EXTRA_CATEGORIES {
    "L44FO_SMC_MM_TEXT_UL",
};


// category hook stuff
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

static const std::vector<category_hierarchy_t> extra_category_hierarchy = {
    {SORT_CUSTOM_ULTIMATE,    SORT_ROOT,            NULL,                      "SMM_T0950_JA.png", NULL, NULL},
    {SORT_CUSTOM_OMNIMIX,     SORT_CUSTOM_ULTIMATE, custom_filter_omnimix,     "SMM_T0960_JA.png", NULL, NULL},
    {SORT_CUSTOM_JUBEAT_PLUS, SORT_CUSTOM_ULTIMATE, custom_filter_jubeat_plus, "SMM_T0961_JA.png", NULL, NULL},
    {SORT_CUSTOM_JUBEAT_2020, SORT_CUSTOM_ULTIMATE, custom_filter_jubeat_2020, "SMM_T0962_JA.png", NULL, NULL},
    {SORT_CUSTOM_JUKEBEAT,    SORT_CUSTOM_ULTIMATE, custom_filter_jukebeat,    "SMM_T0963_JA.png", NULL, NULL},
};

static const std::vector<category_listing_t> extra_category_layout = {
    {SORT_CUSTOM_ULTIMATE, 3, {
        {}, // leave first column blank
        {SORT_CUSTOM_OMNIMIX, SORT_CUSTOM_JUBEAT_PLUS, SORT_CUSTOM_JUBEAT_2020},
        {SORT_CUSTOM_JUKEBEAT},
    }},
};

// clang-format on

static void hook_pkfs_fs_open(HANDLE process, HMODULE pkfs_module)
{
    const IMAGE_IMPORT_DESCRIPTOR *avs2_import_descriptor;
    void *avs_strlcpy_entry_ptr, *avs_strlen_entry_ptr, *avs_snprintf_entry_ptr;
    uint8_t *current;
    size_t remaining;

    avs2_import_descriptor = module_get_iid_for_name(pkfs_module, "avs2-core.dll");

    log_assert(avs2_import_descriptor != nullptr);

    avs_strlcpy_entry_ptr =
        iid_get_addr_for_name(pkfs_module, avs2_import_descriptor, 224, nullptr);
    avs_strlen_entry_ptr = iid_get_addr_for_name(pkfs_module, avs2_import_descriptor, 222, nullptr);
    avs_snprintf_entry_ptr =
        iid_get_addr_for_name(pkfs_module, avs2_import_descriptor, 258, nullptr);

    log_assert(avs_strlcpy_entry_ptr != nullptr);
    log_assert(avs_strlen_entry_ptr != nullptr);
    log_assert(avs_snprintf_entry_ptr != nullptr);

#ifdef VERBOSE
    log_info("avs_strlcpy entry = %p", avs_strlcpy_entry_ptr);
    log_info("avs_strlen entry = %p", avs_strlen_entry_ptr);
    log_info("avs_snprintf entry = %p", avs_snprintf_entry_ptr);
#endif

    // Initialize base patterns
    uint8_t avs_strlcpy_pattern[] = { 0xFF, 0x15, 0x00, 0x00, 0x00, 0x00 };
    uint8_t avs_strlen_pattern[] = { 0xFF, 0x15, 0x00, 0x00, 0x00, 0x00 };
    uint8_t avs_snprintf_pattern[] = { 0x8B, 0x35, 0x00, 0x00, 0x00, 0x00 };

    // Copy in function pointers
    memcpy(&avs_strlcpy_pattern[2], &avs_strlcpy_entry_ptr, 4);
    memcpy(&avs_strlen_pattern[2], &avs_strlen_entry_ptr, 4);
    memcpy(&avs_snprintf_pattern[2], &avs_snprintf_entry_ptr, 4);

    // `pkfs_fs_open` and `pkfs_fs_open_w` are right next to each other, we abuse
    // that fact
    uintptr_t start = reinterpret_cast<uintptr_t>(GetProcAddress(pkfs_module, "pkfs_fs_open"));
    uintptr_t end = reinterpret_cast<uintptr_t>(GetProcAddress(pkfs_module, "pkfs_fs_open_w"));
    size_t total_size = start - end;

    log_assert(start != 0);
    log_assert(end != 0);

    current = reinterpret_cast<uint8_t *>(start);
    remaining = total_size;
    while (current != nullptr) {
        current = find_pattern(
            current, remaining, avs_strlcpy_pattern, nullptr, std::size(avs_strlcpy_pattern));

        if (current != nullptr) {
            remaining = end - reinterpret_cast<uintptr_t>(current);

            do_relative_jmp(process, current, reinterpret_cast<const void *>(pkfs_avs_strlcpy));
        }
    }

    current = reinterpret_cast<uint8_t *>(start);
    remaining = total_size;
    while (current != nullptr) {
        current = find_pattern(
            current, remaining, avs_strlen_pattern, nullptr, std::size(avs_strlen_pattern));

        if (current != nullptr) {
            remaining = end - reinterpret_cast<uintptr_t>(current);

            do_relative_jmp(process, current, reinterpret_cast<const void *>(pkfs_avs_strlen));
        }
    }

    current = reinterpret_cast<uint8_t *>(start);
    remaining = total_size;
    while (current != nullptr) {
        current = find_pattern(
            current, remaining, avs_snprintf_pattern, nullptr, std::size(avs_snprintf_pattern));

        if (current != nullptr) {
            remaining = end - reinterpret_cast<uintptr_t>(current);

            do_absolute_jmp(process, current, reinterpret_cast<uint32_t>(pkfs_avs_snprintf));
        }
    }
}

extern "C" DLL_EXPORT bool __cdecl ultimate_dll_entry_init(char *sid_code, void *app_config)
{
    DWORD pid;
    HANDLE process;
    HMODULE avs2_core_handle, jubeat_handle, music_db_handle, pkfs_handle;
    MODULEINFO jubeat_info, music_db_info;

    log_to_external(log_body_misc, log_body_info, log_body_warning, log_body_fatal);

    log_info("jubeat ultimate hook by Felix, Cannu & mon v" OMNIMIX_VERSION " (Build " __DATE__
             " " __TIME__ ")");

    pid = GetCurrentProcessId();
    process = OpenProcess(
        PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE,
        false,
        pid);

    if ((avs2_core_handle = GetModuleHandleA("avs2-core.dll")) == nullptr) {
        log_fatal("GetModuleHandle(\"avs2-core.dll\") failed: 0x%08lx", GetLastError());
    }
    if ((jubeat_handle = GetModuleHandleA("jubeat.dll")) == nullptr) {
        log_fatal("GetModuleHandle(\"jubeat.dll\") failed: 0x%08lx", GetLastError());
    }
    if ((music_db_handle = GetModuleHandleA("music_db.dll")) == nullptr) {
        log_fatal("GetModuleHandle(\"music_db.dll\") failed: 0x%08lx", GetLastError());
    }
    if ((pkfs_handle = GetModuleHandleA("pkfs.dll")) == nullptr) {
        log_fatal("GetModuleHandle(\"pkfs.dll\") failed: 0x%08lx", GetLastError());
    }

#ifdef VERBOSE
    log_info("avs2-core.dll = %p", reinterpret_cast<void *>(avs2_core_handle));
    log_info("jubeat.dll = %p", reinterpret_cast<void *>(jubeat_handle));
    log_info("music_db.dll = %p", reinterpret_cast<void *>(music_db_handle));
    log_info("pkfs.dll = %p", reinterpret_cast<void *>(pkfs_handle));
    log_info("sid_code = %s", sid_code);
#endif

    if (!GetModuleInformation(process, jubeat_handle, &jubeat_info, sizeof(jubeat_info))) {
        log_fatal("GetModuleInformation(\"jubeat.dll\") failed: 0x%08lx", GetLastError());
    }
    if (!GetModuleInformation(process, music_db_handle, &music_db_info, sizeof(music_db_info))) {
        log_fatal("GetModuleInformation(\"music_db.dll\") failed: 0x%08lx", GetLastError());
    }

    log_info(
        "mdb_arr_patch: 0x%x score_arr_patch: 0x%x end_score_arr_patch: 0x%x",
        mdb_arr_patch,
        score_arr_patch,
        end_score_arr_patch);

    do_patch(process, jubeat_info, packlist_pluslist);
    do_patch(process, jubeat_info, mdb_array_1_0);
    do_patch(process, jubeat_info, mdb_array_1_1);
    do_patch(process, jubeat_info, mdb_array_1_2);
    do_patch(process, jubeat_info, mdb_array_2_0);
    do_patch(process, jubeat_info, mdb_array_2_1);
    do_patch(process, jubeat_info, mdb_array_2_2);
    do_patch(process, jubeat_info, mdb_array_3_0);
    do_patch(process, jubeat_info, mdb_array_3_1);
    do_patch(process, jubeat_info, mdb_array_3_2);
    do_patch(process, jubeat_info, mdb_array_3_3);
    do_patch(process, jubeat_info, mdb_array_3_4);
    do_patch(process, jubeat_info, mdb_array_3_5);
    do_patch(process, jubeat_info, mdb_array_3_6);
    /*
    do_patch(process, jubeat_info, smc_mm_text_ja);
    do_patch(process, jubeat_info, smc_mm_text_ko);
    do_patch(process, jubeat_info, smc_mm_hierarchy_ja);
    do_patch(process, jubeat_info, smc_mm_hierarchy_ko);
    */

    MH_Initialize();

    hook_music_db(process, jubeat_handle, music_db_handle);
    hook_pkfs_fs_open(process, pkfs_handle);
    bnr_hook_init(jubeat_info);
    bnr_hook_add_paths("L44_BNR_BIG_ID99999999", BNR_TEXTURES);
    bnr_hook_add_paths("L44_TM_BANNER", EXTRA_MARKERS);
    bnr_hook_add_paths("L44FO_PLAY_BACKGROUND", EXTRA_BACKGROUNDS);
    bnr_hook_add_paths("L44FO_STG_BG_CHANGE", EXTRA_BG_CHANGES);
    bnr_hook_add_paths("L44FO_SMC_MM_TEXT_JA", EXTRA_CATEGORIES);
    festo_apply_common_patches(process, jubeat_handle, jubeat_info, music_db_info);

    // category stuff
    category_hooks_add_category_definitions(extra_category_hierarchy);
    category_hooks_add_category_layouts(extra_category_layout);
    category_hooks_init(process, jubeat_info);
    auto root = category_hooks_get_listing(SORT_ROOT);
    // add our extension category to the root folder, it has 7 columns by default
    root->column_count++;
    root->columns[7][0] = SORT_CUSTOM_ULTIMATE;

    MH_EnableHook(MH_ALL_HOOKS);

    CloseHandle(process);

    // Prevent the game from overriding the `rev` field
    SetEnvironmentVariableA("MB_MODEL", "----");

    // Call original
    bool ret = dll_entry_init(sid_code, app_config);

    // Set `rev` to indicate ultimate
    sid_code[5] = 'U';

    return ret;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    (void) hinstDLL;
    (void) fdwReason;
    (void) lpvReserved;

    return TRUE;
}
