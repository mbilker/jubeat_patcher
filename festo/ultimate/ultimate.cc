#define LOG_MODULE "ultimate"

#include <vector>

#include <windows.h>
#include <psapi.h>

#include <stdint.h>

#include "imports/avs2-core/avs.h"
#include "imports/gftools.h"
#include "imports/jubeat.h"

#include "pattern/pattern.h"

#include "pe/iat.h"

#include "util/log.h"
#include "util/mem.h"
#include "util/x86.h"

#include "music_db.h"
#include "pkfs.h"

struct patch_t {
    const char *name;
    const std::vector<uint8_t> pattern;
    const bool *pattern_mask;
    const std::vector<uint8_t> data;
    size_t data_offset;
};

const struct patch_t tutorial_skip {
    .name = "tutorial skip",
    .pattern = { 0x3D, 0x21, 0x00, 0x00, 0x80, 0x75, 0x75, 0x56, 0x68, 0x00, 0x00, 0x60, 0x23, 0x57, 0xFF, 0x15 },
    .data = { 0xE9, 0x01, 0x01, 0x00, 0x00 },
    .data_offset = 5,
};

const struct patch_t select_timer_freeze {
    .name = "song select timer freeze",
    .pattern = { 0x74, 0x00, 0xE8, 0x00, 0x00, 0x00, 0x00, 0x84, 0xC0, 0x75, 0x00, 0x38 },
    .pattern_mask = (const bool[]) { 1, 0, 1, 0, 0, 0, 0, 1, 1, 1, 0, 1 },
    .data = { 0xEB },
    .data_offset = 9,
};

const struct patch_t packlist_pluslist {
    .name = "ultilist patch",
    .pattern = { 'p', 'a', 'c', 'k', 'l', 'i', 's', 't' },
    .data = { 'u', 'l', 't', 'i' },
    .data_offset = 0,
};

const struct patch_t music_db_limit_1 {
    .name = "music_db limit patch 1",
    .pattern = { 0x00, 0x00, 0x20, 0x00, 0x57, 0xFF, 0x15 },
    .data = { 0x60 },
    .data_offset = 2,
};
const struct patch_t music_db_limit_2_old {
    .name = "music_db limit patch 2 (old)",
    .pattern = { 0x00, 0x00, 0x20, 0x00, 0x8B, 0xF8, 0x57 },
    .data = { 0x60 },
    .data_offset = 2,
};
const struct patch_t music_db_limit_2_new {
    .name = "music_db limit patch 2 (new)",
    .pattern = { 0x00, 0x00, 0x20, 0x00, 0x53, 0x6A, 0x01 },
    .data = { 0x60 },
    .data_offset = 2,
};
const struct patch_t music_db_limit_3 {
    .name = "music_db limit patch 3",
    .pattern = { 0x00, 0x00, 0x20, 0x00, 0x6A, 0x00, 0xFF },
    .data = { 0x60 },
    .data_offset = 2,
};
const struct patch_t music_db_limit_4 {
    .name = "music_db limit patch 4",
    .pattern = { 0x00, 0x00, 0x20, 0x00, 0x50, 0x6A, 0x17 },
    .data = { 0x60 },
    .data_offset = 2,
};

const struct patch_t music_plus_patch {
    .name = "music_ulti patch",
    .pattern = { 'm', 'u', 's', 'i', 'c', '_', 'i', 'n', 'f', 'o', '.', 'x', 'm', 'l' },
    .data = { 'u', 'l', 't', 'i' },
    .data_offset = 6,
};

const struct patch_t song_unlock_patch {
    .name = "song unlock",
    .pattern = { 0xC4, 0x04, 0x84, 0xC0, 0x74, 0x09 },
    .data = { 0x90, 0x90 },
    .data_offset = 4,
};

static int stack_replacer[MAX_SONGS];
const size_t mdb_arr_patch = (int)stack_replacer;

#define U32_TO_CONST_BYTES_LE(x) \
        (uint8_t)((x) & 0xff), \
        (uint8_t)((x >> 8) & 0xff), \
        (uint8_t)((x >> 16) & 0xff), \
        (uint8_t)((x >> 24) & 0xff)

struct patch_t mdb_array_1_0 {
    .name = "mdb 1.0",
    .pattern = { 0x00, 0x68, 0x00, 0x08, 0x00, 0x00, 0x50 },
    .data =    { U32_TO_CONST_BYTES_LE(MAX_SONGS) },
    .data_offset = 2,
};

struct patch_t mdb_array_1_1 {
    .name = "mdb 1.1",
    .pattern = { 0x8D, 0x84, 0x24, 0x38, 0x02, 0x00, 0x00 },
    .data =    {
        0xB8,
        U32_TO_CONST_BYTES_LE(mdb_arr_patch),
        0x90,
        0x90 },
    .data_offset = 0,
};

struct patch_t mdb_array_1_2 {
    .name = "mdb 1.2",
    .pattern = { 0x8B, 0x8C, 0x84, 0x38, 0x02, 0x00, 0x00 },
    .data =    {
        0x8B,
        0x0C,
        0x85,
        U32_TO_CONST_BYTES_LE(mdb_arr_patch),
    },
    .data_offset = 0,
};

struct patch_t mdb_array_2_0 {
    .name = "mdb 2.0",
    .pattern = { 0x68, 0x00, 0x08, 0x00, 0x00, 0x8D, 0x8C, 0x24, 0x3C },
    .data =    { U32_TO_CONST_BYTES_LE(MAX_SONGS) },
    .data_offset = 1,
};

struct patch_t mdb_array_2_1 {
    .name = "mdb 2.1",
    .pattern = { 0x8D, 0x8C, 0x24, 0x3C, 0x02, 0x00, 0x00 },
    .data =    {
        0xB9,
        U32_TO_CONST_BYTES_LE(mdb_arr_patch),
        0x90,
        0x90
    },
    .data_offset = 0,
};

struct patch_t mdb_array_2_2 {
    .name = "mdb 2.2",
    .pattern = { 0x8B, 0x94, 0x9C, 0x38, 0x02, 0x00, 0x00 },
    .data =    {
        0x8B,
        0x14,
        0x9D,
        U32_TO_CONST_BYTES_LE(mdb_arr_patch),
    },
    .data_offset = 0,
};

static void *D3_PACKAGE_LOAD = nullptr;
static const char *BNR_TEXTURES[] = {
    "L44FO_BNR_J_01_001",
    "L44FO_BNR_J_02_001",
    "L44FO_BNR_J_03_001",
    "L44FO_BNR_J_04_001",
    "L44FO_BNR_J_05_001",
	"L44FO_BNR_J_05_002",
    "L44FO_BNR_J_06_001",
    "L44FO_BNR_J_07_001",
    "L44FO_BNR_J_08_001",
    "L44FO_BNR_J_09_001",
    "L44FO_BNR_J_09_002",
    "L44FO_BNR_J_09_003",
    "L44FO_BNR_J_09_004",
    "L44FO_BNR_J_09_005",
    "L44FO_BNR_J_09_006",
    "L44FO_BNR_J_09_007",
    "L44FO_BNR_J_09_008",
    "L44FO_BNR_J_09_009",
    "L44FO_BNR_J_09_010",
    "L44FO_BNR_J_09_011",
    "L44FO_BNR_J_09_012",
    "L44FO_BNR_J_09_014",
    "L44FO_BNR_J_OM_MIS",
    "L44FO_BNR_J_EX_001",
    "L44FO_BNR_J_EX_002",
    "L44FO_BNR_J_EX_003",
    "L44FO_BNR_J_EX_004",
    "L44FO_BNR_J_99_999",
    "L44_BNR_BIG_ID99999999"
};

static void do_patch(HANDLE process, const MODULEINFO &module_info, const struct patch_t &patch) {
#ifdef VERBOSE
    char *hex_data;
#endif
    uint8_t *addr, *target;

#ifdef VERBOSE
    log_info("===== %s =====", patch.name);

    hex_data = to_hex(patch.pattern.data(), patch.pattern.size());
    log_info("pattern: %s", hex_data);
    free(hex_data);

    if (patch.pattern_mask != NULL) {
        hex_data = to_hex(reinterpret_cast<const uint8_t *>(patch.pattern_mask), patch.pattern.size());
        log_info("mask   : %s", hex_data);
        free(hex_data);
    }
#endif

    addr = find_pattern(
            reinterpret_cast<uint8_t *>(module_info.lpBaseOfDll),
            module_info.SizeOfImage,
            patch.pattern.data(),
            patch.pattern_mask,
            patch.pattern.size());

    if (addr != NULL) {
#ifdef VERBOSE
        hex_data = to_hex(addr, patch.pattern.size());
        log_info("data: %s", hex_data);
        free(hex_data);
#endif

        target = &addr[patch.data_offset];

        memory_write(process, target, patch.data);

#ifdef VERBOSE
        log_info("%s applied at %p", patch.name, target);

        hex_data = to_hex(addr, patch.pattern.size());
        log_info("data: %s", hex_data);
        free(hex_data);
#endif
    } else {
        log_warning("could not find %s base address", patch.name);
    }
}

static void hook_pkfs_fs_open(
        HANDLE process,
        HMODULE pkfs_module,
        const MODULEINFO &module_info)
{
    const IMAGE_IMPORT_DESCRIPTOR *avs2_import_descriptor;
    void *avs_strlcpy_entry_ptr, *avs_strlen_entry_ptr, *avs_snprintf_entry_ptr;
    uint8_t *current;
    size_t remaining;

    avs2_import_descriptor = module_get_iid_for_name(process, pkfs_module, "avs2-core.dll");

    log_assert(avs2_import_descriptor != nullptr);

    avs_strlcpy_entry_ptr = iid_get_addr_for_name(pkfs_module, avs2_import_descriptor, 224, nullptr);
    avs_strlen_entry_ptr = iid_get_addr_for_name(pkfs_module, avs2_import_descriptor, 222, nullptr);
    avs_snprintf_entry_ptr = iid_get_addr_for_name(pkfs_module, avs2_import_descriptor, 258, nullptr);

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

    // `pkfs_fs_open` and `pkfs_fs_open_w` are right next to each other, we abuse that fact
    uintptr_t start = reinterpret_cast<uintptr_t>(GetProcAddress(pkfs_module, "pkfs_fs_open"));
    uintptr_t end = reinterpret_cast<uintptr_t>(GetProcAddress(pkfs_module, "pkfs_fs_open_w"));
    size_t total_size = start - end;

    log_assert(start != 0);
    log_assert(end != 0);

    current = reinterpret_cast<uint8_t *>(start);
    remaining = total_size;
    while (current != nullptr) {
        current = find_pattern(
                current,
                remaining,
                avs_strlcpy_pattern,
                nullptr,
                std::size(avs_strlcpy_pattern));

        if (current != nullptr) {
            remaining = end - reinterpret_cast<uintptr_t>(current);

            do_relative_jmp(process, current, reinterpret_cast<const void *>(pkfs_avs_strlcpy));
        }
    }

    current = reinterpret_cast<uint8_t *>(start);
    remaining = total_size;
    while (current != nullptr) {
        current = find_pattern(
                current,
                remaining,
                avs_strlen_pattern,
                nullptr,
                std::size(avs_strlen_pattern));

        if (current != nullptr) {
            remaining = end - reinterpret_cast<uintptr_t>(current);

            do_relative_jmp(process, current, reinterpret_cast<const void *>(pkfs_avs_strlen));
        }
    }

    current = reinterpret_cast<uint8_t *>(start);
    remaining = total_size;
    while (current != nullptr) {
        current = find_pattern(
                current,
                remaining,
                avs_snprintf_pattern,
                nullptr,
                std::size(avs_snprintf_pattern));

        if (current != nullptr) {
            remaining = end - reinterpret_cast<uintptr_t>(current);

            do_absolute_jmp(process, current, reinterpret_cast<uint32_t>(pkfs_avs_snprintf));
        }
    }
}

static void __cdecl banner_load_hook() {
    for (const char *bnr_package : BNR_TEXTURES) {
        __asm__(
            "push esp\n"
            "mov ecx, %0\n"
            "call %1\n"
            "pop esp"
            :
            : "r" (bnr_package), "r" (D3_PACKAGE_LOAD)
            // 2020021900 saves `ebx`, `ebp`, `edi`, and `esi` in `D3_PACKAGE_LOAD`
            : "eax", "ecx", "edx"
        );
    }
}

static void hook_banner_textures(HANDLE process, const MODULEINFO &module_info) {
    // Unique pattern for prologue for banner texture loading
    // add esp, 12
    // mov ecx, 12
    const uint8_t prologue_pattern[] = { 0x83, 0xC4, 0x0C, 0xB9, 0x0C, 0x00, 0x00, 0x00 };

    // Pattern to get the `jz` for the loop
    const uint8_t loop_jz_pattern[] = { 0x85, 0xC0, 0x74 };

    // Pattern to get the `inc`, `test`, `jnz` portion of the loop
    const uint8_t loop_jnz_pattern[] = { 0x46, 0x85, 0xC0, 0x75 };

    // Pattern to get the D3 texture load function address
    const uint8_t d3_package_load_pattern[] = { 0x8B, 0xC8, 0xE8, 0x00, 0x00, 0x00, 0x00 };
    const bool d3_package_load_pattern_mask[] = { 1, 1, 1, 0, 0, 0, 0 };

    void *base_addr = find_pattern(
            reinterpret_cast<uint8_t *>(module_info.lpBaseOfDll),
            module_info.SizeOfImage,
            prologue_pattern,
            nullptr,
            std::size(prologue_pattern));

    log_assert(base_addr != nullptr);

    void *loop_jz_addr = find_pattern(
            reinterpret_cast<uint8_t *>(base_addr),
            module_info.SizeOfImage - reinterpret_cast<uintptr_t>(base_addr),
            loop_jz_pattern,
            nullptr,
            std::size(loop_jz_pattern));

    log_assert(loop_jz_addr != nullptr);

    // Offset to actual `jz` instruction from base of pattern
    loop_jz_addr = reinterpret_cast<uint8_t *>(loop_jz_addr) + 2;

    void *loop_jnz_addr = find_pattern(
            reinterpret_cast<uint8_t *>(loop_jz_addr),
            module_info.SizeOfImage - reinterpret_cast<uintptr_t>(loop_jz_addr),
            loop_jnz_pattern,
            nullptr,
            std::size(loop_jnz_pattern));

    log_assert(loop_jnz_addr != nullptr);

    void *d3_package_load_call_addr = find_pattern(
            reinterpret_cast<uint8_t *>(loop_jz_addr),
            reinterpret_cast<uintptr_t>(loop_jnz_addr) - reinterpret_cast<uintptr_t>(loop_jz_addr),
            d3_package_load_pattern,
            d3_package_load_pattern_mask,
            std::size(d3_package_load_pattern));

    log_assert(d3_package_load_call_addr != nullptr);

    // Save the address of the original function
    {
        // Compute address to the `E8 xx xx xx xx` (relative jump) instruction
        uintptr_t jump_addr = reinterpret_cast<uintptr_t>(d3_package_load_call_addr) + 2;

        // Load the offset from the relative jump instruction
        uint32_t offset = *reinterpret_cast<uint32_t *>(jump_addr + 1);

        // Compute and save the absolute address of the function for later
        D3_PACKAGE_LOAD = reinterpret_cast<void *>(jump_addr + 5 + offset);
    }

    // Patch function call location
    {
        struct call_replacement {
            uint8_t load_opcode;
            uint32_t addr;
            uint8_t call_opcode;
            uint8_t reg_index;
        } __attribute__((packed));

        struct call_replacement d3_package_load_call_replacement {
            .load_opcode = 0xB8,
            .addr = reinterpret_cast<uint32_t>(banner_load_hook),
            .call_opcode = 0xFF,
            .reg_index = 0xD0,
        };
        static_assert(sizeof(d3_package_load_call_replacement) == sizeof(d3_package_load_pattern));

        memory_write(
                process,
                d3_package_load_call_addr,
                &d3_package_load_call_replacement,
                sizeof(d3_package_load_call_replacement));
    }

    // `nop` out the loop increment and jump part (adding one to include the jump target)
    memory_set(process, loop_jnz_addr, 0x90, sizeof(loop_jnz_pattern) + 1);
}

extern "C" bool __declspec(dllexport) dll_entry_init(char *sid_code, void *app_config) {
    DWORD pid;
    HANDLE process;
    HMODULE avs2_core_handle, jubeat_handle, music_db_handle, pkfs_handle;
    MODULEINFO jubeat_info, music_db_info, pkfs_info;

    log_to_external(log_body_misc, log_body_info, log_body_warning, log_body_fatal);

    log_info("jubeat ultimate hook by Felix & mon v" OMNIMIX_VERSION " (Build " __DATE__ " " __TIME__ ")");

    pid = GetCurrentProcessId();
    process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE, false, pid);

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
    if (!GetModuleInformation(process, pkfs_handle, &pkfs_info, sizeof(pkfs_info))) {
        log_fatal("GetModuleInformation(\"pkfs.dll\") failed: 0x%08lx", GetLastError());
    }

    log_body_info("ultimate", "patch array lives at %X", mdb_arr_patch);
    do_patch(process, jubeat_info, tutorial_skip);
    do_patch(process, jubeat_info, select_timer_freeze);
    do_patch(process, jubeat_info, packlist_pluslist);
    do_patch(process, jubeat_info, mdb_array_1_0);
    do_patch(process, jubeat_info, mdb_array_1_1);
    do_patch(process, jubeat_info, mdb_array_1_2);
    do_patch(process, jubeat_info, mdb_array_2_0);
    do_patch(process, jubeat_info, mdb_array_2_1);
    do_patch(process, jubeat_info, mdb_array_2_2);
    // do_patch(process, music_db_info, music_db_limit_1);
    // do_patch(process, music_db_info, music_db_limit_2_old);
    // do_patch(process, music_db_info, music_db_limit_2_new);
    // do_patch(process, music_db_info, music_db_limit_3);
    // do_patch(process, music_db_info, music_db_limit_4);
    // do_patch(process, music_db_info, music_plus_patch);
    do_patch(process, music_db_info, song_unlock_patch);

    hook_iat(
            process,
            jubeat_handle,
            "music_db.dll",
            "music_db_get_sequence_filename",
            reinterpret_cast<void *>(music_db_get_sequence_filename));
    hook_iat(
            process,
            jubeat_handle,
            "music_db.dll",
            "music_db_get_sound_filename",
            reinterpret_cast<void *>(music_db_get_sound_filename));

    music_db_initialize_orig = (bool (*)())GetProcAddress(music_db_handle, "music_db_initialize");

    // mem_set
    hook_iat_ordinal(process, jubeat_handle, "avs2-core.dll", 0xF4, reinterpret_cast<void *>(mem_set));

    //hook_iat(process, jubeat_handle, "music_db.dll", "music_db_dbg_get_all_list", reinterpret_cast<void *>(music_db_dbg_get_all_list));
    //hook_iat(process, jubeat_handle, "music_db.dll", "music_db_dot_array_to_music_bar", reinterpret_cast<void *>(music_db_dot_array_to_music_bar));
    //hook_iat(process, jubeat_handle, "music_db.dll", "music_db_finalize", reinterpret_cast<void *>(music_db_finalize));
    hook_iat(process, jubeat_handle, "music_db.dll", "music_db_get_bpm", reinterpret_cast<void *>(music_db_get_bpm));
    hook_iat(process, jubeat_handle, "music_db.dll", "music_db_get_bpm_min", reinterpret_cast<void *>(music_db_get_bpm_min));
    hook_iat(process, jubeat_handle, "music_db.dll", "music_db_get_default_list", reinterpret_cast<void *>(music_db_get_default_list));
    hook_iat(process, jubeat_handle, "music_db.dll", "music_db_get_offline_default_list", reinterpret_cast<void *>(music_db_get_offline_default_list));
    hook_iat(process, jubeat_handle, "music_db.dll", "music_db_get_all_permitted_list", reinterpret_cast<void *>(music_db_get_all_permitted_list));
    hook_iat(process, jubeat_handle, "music_db.dll", "music_db_get_possession_list", reinterpret_cast<void *>(music_db_get_possession_list));
    hook_iat(process, jubeat_handle, "music_db.dll", "music_db_get_card_default_list", reinterpret_cast<void *>(music_db_get_card_default_list));
    //hook_iat(process, jubeat_handle, "music_db.dll", "music_db_get_jukebox_list", reinterpret_cast<void *>(music_db_get_jukebox_list));
    // hook_iat(process, jubeat_handle, "music_db.dll", "music_db_get_default_id", reinterpret_cast<void *>(music_db_get_default_id));
    // hook_iat(process, jubeat_handle, "music_db.dll", "music_db_get_default_id_by_genre", reinterpret_cast<void *>(music_db_get_default_id_by_genre));
    // hook_iat(process, jubeat_handle, "music_db.dll", "music_db_get_default_id_by_mode", reinterpret_cast<void *>(music_db_get_default_id_by_mode));
    hook_iat(process, jubeat_handle, "music_db.dll", "music_db_get_genre_list", reinterpret_cast<void *>(music_db_get_genre_list));
    hook_iat(process, jubeat_handle, "music_db.dll", "music_db_get_grouping_category_list", reinterpret_cast<void *>(music_db_get_grouping_category_list));
    hook_iat(process, jubeat_handle, "music_db.dll", "music_db_get_index_start", reinterpret_cast<void *>(music_db_get_index_start));
    hook_iat(process, jubeat_handle, "music_db.dll", "music_db_get_level", reinterpret_cast<void *>(music_db_get_level));
    hook_iat(process, jubeat_handle, "music_db.dll", "music_db_get_level_detail", reinterpret_cast<void *>(music_db_get_level_detail));
    hook_iat(process, jubeat_handle, "music_db.dll", "music_db_get_music_name_head_index", reinterpret_cast<void *>(music_db_get_music_name_head_index));
    hook_iat(process, jubeat_handle, "music_db.dll", "music_db_get_music_name_index", reinterpret_cast<void *>(music_db_get_music_name_index));
    hook_iat(process, jubeat_handle, "music_db.dll", "music_db_get_parent_music_id", reinterpret_cast<void *>(music_db_get_parent_music_id));
    hook_iat(process, jubeat_handle, "music_db.dll", "music_db_get_permitted_music_flag", reinterpret_cast<void *>(music_db_get_permitted_music_flag));
    hook_iat(process, jubeat_handle, "music_db.dll", "music_db_get_pos_index", reinterpret_cast<void *>(music_db_get_pos_index));
    hook_iat(process, jubeat_handle, "music_db.dll", "music_db_initialize", reinterpret_cast<void *>(music_db_initialize));
    //hook_iat(process, jubeat_handle, "music_db.dll", "music_db_is_all_yellow", reinterpret_cast<void *>(music_db_is_all_yellow));
    //hook_iat(process, jubeat_handle, "music_db.dll", "music_db_is_displayable_level_detail", reinterpret_cast<void *>(music_db_is_displayable_level_detail));
    hook_iat(process, jubeat_handle, "music_db.dll", "music_db_is_exists_table", reinterpret_cast<void *>(music_db_is_exists_table));
    hook_iat(process, jubeat_handle, "music_db.dll", "music_db_is_exists_version_from_ver1", reinterpret_cast<void *>(music_db_is_exists_version_from_ver1));
    hook_iat(process, jubeat_handle, "music_db.dll", "music_db_is_exists_version_from_ver2", reinterpret_cast<void *>(music_db_is_exists_version_from_ver2));
    hook_iat(process, jubeat_handle, "music_db.dll", "music_db_is_exists_version_from_ver3", reinterpret_cast<void *>(music_db_is_exists_version_from_ver3));
    hook_iat(process, jubeat_handle, "music_db.dll", "music_db_is_exists_version_from_ver4", reinterpret_cast<void *>(music_db_is_exists_version_from_ver4));
    hook_iat(process, jubeat_handle, "music_db.dll", "music_db_is_exists_version_from_ver5", reinterpret_cast<void *>(music_db_is_exists_version_from_ver5));
    hook_iat(process, jubeat_handle, "music_db.dll", "music_db_is_exists_version_from_ver5_5", reinterpret_cast<void *>(music_db_is_exists_version_from_ver5_5));
    hook_iat(process, jubeat_handle, "music_db.dll", "music_db_is_exists_version_from_ver6", reinterpret_cast<void *>(music_db_is_exists_version_from_ver6));
    hook_iat(process, jubeat_handle, "music_db.dll", "music_db_is_exists_version_from_ver7", reinterpret_cast<void *>(music_db_is_exists_version_from_ver7));
    hook_iat(process, jubeat_handle, "music_db.dll", "music_db_is_exists_version_from_ver8", reinterpret_cast<void *>(music_db_is_exists_version_from_ver8));
    hook_iat(process, jubeat_handle, "music_db.dll", "music_db_is_exists_version_from_ver9", reinterpret_cast<void *>(music_db_is_exists_version_from_ver9));
    hook_iat(process, jubeat_handle, "music_db.dll", "music_db_is_hold_marker", reinterpret_cast<void *>(music_db_is_hold_marker));
    hook_iat(process, jubeat_handle, "music_db.dll", "music_db_is_matched_select_type", reinterpret_cast<void *>(music_db_is_matched_select_type));
    //hook_iat(process, jubeat_handle, "music_db.dll", "music_db_is_matching_select", reinterpret_cast<void *>(music_db_is_matching_select));
    //hook_iat(process, jubeat_handle, "music_db.dll", "music_db_is_nearly_excellent", reinterpret_cast<void *>(music_db_is_nearly_excellent));
    //hook_iat(process, jubeat_handle, "music_db.dll", "music_db_is_nearly_fullcombo", reinterpret_cast<void *>(music_db_is_nearly_fullcombo));
    //hook_iat(process, jubeat_handle, "music_db.dll", "music_db_is_new", reinterpret_cast<void *>(music_db_is_new));
    //hook_iat(process, jubeat_handle, "music_db.dll", "music_db_is_no_gray", reinterpret_cast<void *>(music_db_is_no_gray));
    hook_iat(process, jubeat_handle, "music_db.dll", "music_db_is_permitted", reinterpret_cast<void *>(music_db_is_permitted));
    hook_iat(process, jubeat_handle, "music_db.dll", "music_db_is_possession_for_contained_music_list", reinterpret_cast<void *>(music_db_is_possession_for_contained_music_list));
    //hook_iat(process, jubeat_handle, "music_db.dll", "music_db_is_random_or_matching_select", reinterpret_cast<void *>(music_db_is_random_or_matching_select));
    //hook_iat(process, jubeat_handle, "music_db.dll", "music_db_is_random_select", reinterpret_cast<void *>(music_db_is_random_select));
    //hook_iat(process, jubeat_handle, "music_db.dll", "music_db_music_bar_to_dot_array", reinterpret_cast<void *>(music_db_music_bar_to_dot_array));
    //hook_iat(process, jubeat_handle, "music_db.dll", "music_db_reset_using_datapackage", reinterpret_cast<void *>(music_db_reset_using_datapackage));
    // hook_iat(process, jubeat_handle, "music_db.dll", "music_db_set_default_add_music_flag", reinterpret_cast<void *>(music_db_set_default_add_music_flag));
    // hook_iat(process, jubeat_handle, "music_db.dll", "music_db_set_flag_equivalent_for_music_id", reinterpret_cast<void *>(music_db_set_flag_equivalent_for_music_id));
    // hook_iat(process, jubeat_handle, "music_db.dll", "music_db_set_permitted_music_flag", reinterpret_cast<void *>(music_db_set_permitted_music_flag));
    //hook_iat(process, jubeat_handle, "music_db.dll", "music_db_set_select_history_list", reinterpret_cast<void *>(music_db_set_select_history_list));

    // hook_iat(process, jubeat_handle, "music_db.dll", "music_bonus_get_bonus_music", reinterpret_cast<void *>(music_bonus_get_bonus_music));
    // hook_iat(process, jubeat_handle, "music_db.dll", "music_bonus_is_bonus_music", reinterpret_cast<void *>(music_bonus_is_bonus_music));
    // hook_iat(process, jubeat_handle, "music_db.dll", "music_only_now_get_count", reinterpret_cast<void *>(music_only_now_get_count));
    // hook_iat(process, jubeat_handle, "music_db.dll", "music_only_now_get_etime", reinterpret_cast<void *>(music_only_now_get_etime));
    // hook_iat(process, jubeat_handle, "music_db.dll", "music_only_now_get_music_id", reinterpret_cast<void *>(music_only_now_get_music_id));
    // hook_iat(process, jubeat_handle, "music_db.dll", "music_only_now_initialize", reinterpret_cast<void *>(music_only_now_initialize));

    // TODO: all music_record function hooks are broken
    // hook_iat(process, jubeat_handle, "music_db.dll", "music_record_add_clear_count", reinterpret_cast<void *>(music_record_add_clear_count));
    // hook_iat(process, jubeat_handle, "music_db.dll", "music_record_add_excellent_count", reinterpret_cast<void *>(music_record_add_excellent_count));
    // hook_iat(process, jubeat_handle, "music_db.dll", "music_record_add_full_combo_count", reinterpret_cast<void *>(music_record_add_full_combo_count));
    // hook_iat(process, jubeat_handle, "music_db.dll", "music_record_add_play_count", reinterpret_cast<void *>(music_record_add_play_count));
    // hook_iat(process, jubeat_handle, "music_db.dll", "music_record_clear_context", reinterpret_cast<void *>(music_record_clear_context));
    // hook_iat(process, jubeat_handle, "music_db.dll", "music_record_get_best_music_rate", reinterpret_cast<void *>(music_record_get_best_music_rate));
    // hook_iat(process, jubeat_handle, "music_db.dll", "music_record_get_best_score", reinterpret_cast<void *>(music_record_get_best_score));
    // hook_iat(process, jubeat_handle, "music_db.dll", "music_record_get_clear_count", reinterpret_cast<void *>(music_record_get_clear_count));
    // hook_iat(process, jubeat_handle, "music_db.dll", "music_record_get_clear_flag", reinterpret_cast<void *>(music_record_get_clear_flag));
    // hook_iat(process, jubeat_handle, "music_db.dll", "music_record_get_excellent_count", reinterpret_cast<void *>(music_record_get_excellent_count));
    // hook_iat(process, jubeat_handle, "music_db.dll", "music_record_get_full_combo_count", reinterpret_cast<void *>(music_record_get_full_combo_count));
    // hook_iat(process, jubeat_handle, "music_db.dll", "music_record_get_jubility", reinterpret_cast<void *>(music_record_get_jubility));
    // hook_iat(process, jubeat_handle, "music_db.dll", "music_record_get_music_bar", reinterpret_cast<void *>(music_record_get_music_bar));
    // hook_iat(process, jubeat_handle, "music_db.dll", "music_record_get_play_count", reinterpret_cast<void *>(music_record_get_play_count));
    // hook_iat(process, jubeat_handle, "music_db.dll", "music_record_get_sequence_record_set", reinterpret_cast<void *>(music_record_get_sequence_record_set));
    // hook_iat(process, jubeat_handle, "music_db.dll", "music_record_get_total_best_score", reinterpret_cast<void *>(music_record_get_total_best_score));
    // hook_iat(process, jubeat_handle, "music_db.dll", "music_record_is_all_played", reinterpret_cast<void *>(music_record_is_all_played));
    // hook_iat(process, jubeat_handle, "music_db.dll", "music_record_is_all_yellow", reinterpret_cast<void *>(music_record_is_all_yellow));
    // hook_iat(process, jubeat_handle, "music_db.dll", "music_record_is_any_played", reinterpret_cast<void *>(music_record_is_any_played));
    // hook_iat(process, jubeat_handle, "music_db.dll", "music_record_is_cleared", reinterpret_cast<void *>(music_record_is_cleared));
    // hook_iat(process, jubeat_handle, "music_db.dll", "music_record_is_excellent", reinterpret_cast<void *>(music_record_is_excellent));
    // hook_iat(process, jubeat_handle, "music_db.dll", "music_record_is_full_combo", reinterpret_cast<void *>(music_record_is_full_combo));
    // hook_iat(process, jubeat_handle, "music_db.dll", "music_record_is_no_gray", reinterpret_cast<void *>(music_record_is_no_gray));
    // hook_iat(process, jubeat_handle, "music_db.dll", "?music_record_is_played@@YA_NIEW4MUSIC_RECORD_TYPE@@@Z", reinterpret_cast<void *>(music_record_is_played));
    // hook_iat(process, jubeat_handle, "music_db.dll", "music_record_merge_music_bar", reinterpret_cast<void *>(music_record_merge_music_bar));
    // hook_iat(process, jubeat_handle, "music_db.dll", "music_record_set_best_music_rate", reinterpret_cast<void *>(music_record_set_best_music_rate));
    // hook_iat(process, jubeat_handle, "music_db.dll", "music_record_set_best_score", reinterpret_cast<void *>(music_record_set_best_score));
    // hook_iat(process, jubeat_handle, "music_db.dll", "music_record_set_clear_count", reinterpret_cast<void *>(music_record_set_clear_count));
    // hook_iat(process, jubeat_handle, "music_db.dll", "music_record_set_clear_flag", reinterpret_cast<void *>(music_record_set_clear_flag));
    // hook_iat(process, jubeat_handle, "music_db.dll", "music_record_set_excellent_count", reinterpret_cast<void *>(music_record_set_excellent_count));
    // hook_iat(process, jubeat_handle, "music_db.dll", "music_record_set_full_combo_count", reinterpret_cast<void *>(music_record_set_full_combo_count));
    // hook_iat(process, jubeat_handle, "music_db.dll", "music_record_set_jubility", reinterpret_cast<void *>(music_record_set_jubility));
    // hook_iat(process, jubeat_handle, "music_db.dll", "music_record_set_play_count", reinterpret_cast<void *>(music_record_set_play_count));
    // hook_iat(process, jubeat_handle, "music_db.dll", "music_shareable_add_shareable_music", reinterpret_cast<void *>(music_shareable_add_shareable_music));
    // hook_iat(process, jubeat_handle, "music_db.dll", "music_shareable_initialize", reinterpret_cast<void *>(music_shareable_initialize));
    // hook_iat(process, jubeat_handle, "music_db.dll", "music_shareable_is_shareable_music", reinterpret_cast<void *>(music_shareable_is_shareable_music));
    // hook_iat(process, jubeat_handle, "music_db.dll", "music_shareable_set_flag", reinterpret_cast<void *>(music_shareable_set_flag));

    // hook_iat(process, jubeat_handle, "music_db.dll", "?GetInstance@BlackJacket@music_texture@@SAAAV12@XZ", reinterpret_cast<void *>(music_texture_BlackJacket_GetInstance));
    // hook_iat(process, jubeat_handle, "music_db.dll", "?ReadXmlNode@BlackJacket@music_texture@@QAE_NPAUT_PROPERTY_NODE@@@Z", reinterpret_cast<void *>(music_texture_BlackJacket_ReadXmlNode));
    // hook_iat(process, jubeat_handle, "music_db.dll", "?music_bonus_get_target_music_info@music_bonus_weekly@@YAXPBUJBMusicFlag_T@@AA_NAAI@Z", reinterpret_cast<void *>(music_bonus_weekly_music_bonus_get_target_music_info));
    // hook_iat(process, jubeat_handle, "music_db.dll", "?clear@music_bonus_weekly@@YAXXZ", reinterpret_cast<void *>(music_bonus_weekly_clear));
    // hook_iat(process, jubeat_handle, "music_db.dll", "?music_bonus_weekly_is_target_music@music_bonus_weekly@@YA_NPBUJBMusicFlag_T@@I@Z", reinterpret_cast<void *>(music_bonus_weekly_music_bonus_weekly_is_target_music));
    // hook_iat(process, jubeat_handle, "music_db.dll", "?clear@music_new@@YAXXZ", reinterpret_cast<void *>(music_new_clear));
    // hook_iat(process, jubeat_handle, "music_db.dll", "?get_list@music_new@@YAHHQAI@Z", reinterpret_cast<void *>(music_new_get_list));
    // hook_iat(process, jubeat_handle, "music_db.dll", "?read_xml_node@music_new@@YA_NPAUT_PROPERTY_NODE@@@Z", reinterpret_cast<void *>(music_new_read_xml_node));

    hook_iat(process, music_db_handle, "gftools.dll", "GFHashMapRegist", reinterpret_cast<void *>(GFHashMapRegist));
    hook_iat(process, music_db_handle, "gftools.dll", "GFHashMapCreate", reinterpret_cast<void *>(GFHashMapCreate));
    hook_iat(process, music_db_handle, "gftools.dll", "GFHashMapKeyToValue", reinterpret_cast<void *>(GFHashMapKeyToValue));
    hook_iat(process, music_db_handle, "gftools.dll", "GFHashMapGetEntryList", reinterpret_cast<void *>(GFHashMapGetEntryList));
    hook_iat(process, music_db_handle, "gftools.dll", "GFHashMapRewindEntryList", reinterpret_cast<void *>(GFHashMapRewindEntryList));

    hook_pkfs_fs_open(process, pkfs_handle, pkfs_info);
    hook_banner_textures(process, jubeat_info);

    CloseHandle(process);

    // Prevent the game from overriding the `rev` field
    SetEnvironmentVariableA("MB_MODEL", "----");

    // Call original
    bool ret = jb_dll_entry_init(sid_code, app_config);

    // Set `rev` to indicate ultimate
    sid_code[5] = 'U';

    return ret;

}

extern "C" bool __declspec(dllexport) dll_entry_main(void) {
    return jb_dll_entry_main();
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    return TRUE;
}
