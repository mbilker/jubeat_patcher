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

#include "music_db.h"
#include "pkfs.h"

struct patch_t {
    const char *name;
    const std::vector<uint8_t> pattern;
    // MSVC does not allow the `(const bool[]) { ... }` initializer and `std::vector<bool>` is a
    // specialization, so use `std::vector<uint8_t>` instead
    const std::vector<uint8_t> pattern_mask;
    const std::vector<uint8_t> data;
    size_t data_offset;
};

// clang-format off

const struct patch_t tutorial_skip {
    .name = "tutorial skip",
    .pattern = { 0x3D, 0x21, 0x00, 0x00, 0x80, 0x75, 0x75, 0x56, 0x68, 0x00, 0x00, 0x60, 0x23, 0x57, 0xFF, 0x15 },
    .data = { 0xEB },
    .data_offset = 5,
};

const struct patch_t tutorial_skip2 {
    .name = "tutorial skip 2",
    .pattern = { 0x3D, 0x22, 0x00, 0x00, 0x80, 0x75, 0x31, 0x56, 0x68, 0x00, 0x00, 0x60, 0x23 },
    .data = { 0xEB },
    .data_offset = 5,
};

const struct patch_t select_timer_freeze {
    .name = "song select timer freeze",
    .pattern = { 0x74, 0x00, 0xE8, 0x00, 0x00, 0x00, 0x00, 0x84, 0xC0, 0x75, 0x00, 0x38 },
    .pattern_mask = { 1, 0, 1, 0, 0, 0, 0, 1, 1, 1, 0, 1 },
    .data = { 0xEB },
    .data_offset = 9,
};

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

const struct patch_t song_unlock_patch {
    .name = "song unlock",
    .pattern = { 0xC4, 0x04, 0x84, 0xC0, 0x74, 0x09 },
    .data = { 0x90, 0x90 },
    .data_offset = 4,
};

// clang-format on

static int stack_replacer[MAX_SONGS];
const size_t mdb_arr_patch = (size_t) stack_replacer;

static uint8_t score_stack_replacer[MAX_SONGS][20];
const size_t score_arr_patch = (size_t) score_stack_replacer;
// the asm to replace needs the end of the array
const size_t end_score_arr_patch = (size_t)(&score_stack_replacer[MAX_SONGS]);

// clang-format off

#define U32_TO_CONST_BYTES_LE(x) \
    (uint8_t) ((x) & 0xff), \
    (uint8_t) (((x) >> 8) & 0xff), \
    (uint8_t) (((x) >> 16) & 0xff), \
    (uint8_t) (((x) >> 24) & 0xff)

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
    "L44FO_BNR_J_09_015",
    "L44FO_BNR_J_09_016",
    "L44FO_BNR_J_09_017",
    "L44FO_BNR_J_09_018",
    "L44FO_BNR_J_09_019",
    "L44FO_BNR_J_09_020",
    "L44FO_BNR_J_OM_001",
    "L44FO_BNR_J_OM_002",
    "L44FO_BNR_J_EX_001",
    "L44FO_BNR_J_EX_002",
    "L44FO_BNR_J_EX_003",
    "L44FO_BNR_J_EX_004",
    "L44FO_BNR_J_99_999",
    "L44_BNR_BIG_ID99999999",
};

// clang-format on

static void do_patch(HANDLE process, const MODULEINFO &module_info, const struct patch_t &patch)
{
#ifdef VERBOSE
    char *hex_data;
#endif
    uint8_t *addr, *target;

#ifdef VERBOSE
    log_info("===== %s =====", patch.name);

    hex_data = to_hex(patch.pattern.data(), patch.pattern.size());
    log_info("pattern: %s", hex_data);
    free(hex_data);

    if (!patch.pattern_mask.empty()) {
        hex_data = to_hex(patch.pattern_mask.data(), patch.pattern.size());
        log_info("mask   : %s", hex_data);
        free(hex_data);
    }
#endif

    addr = find_pattern(
        reinterpret_cast<uint8_t *>(module_info.lpBaseOfDll),
        module_info.SizeOfImage,
        patch.pattern.data(),
        reinterpret_cast<const bool *>(patch.pattern_mask.data()),
        patch.pattern.size());

    if (addr != nullptr) {
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

static void hook_pkfs_fs_open(HANDLE process, HMODULE pkfs_module, const MODULEINFO &module_info)
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

static void __cdecl banner_load_hook()
{
    for (const char *bnr_package : BNR_TEXTURES) {
#ifdef _MSC_VER
        __asm {
            // These registers are overwritten
            push eax
            push ecx
            push edx

            // `esp` is used by `D3_PACKAGE_LOAD`
            push esp
            mov ecx, bnr_package
            call D3_PACKAGE_LOAD
            pop esp

            // Restore overwritten registers
            pop edx
            pop ecx
            pop eax
        }
#else
        __asm__(".intel_syntax\n"
                "push esp\n"
                "mov ecx, %0\n"
                "call %1\n"
                "pop esp\n"
                ".att_syntax\n"
                :
                : "r"(bnr_package), "r"(D3_PACKAGE_LOAD)
                // 2020021900 saves `ebx`, `ebp`, `edi`, and `esi` in `D3_PACKAGE_LOAD`
                : "eax", "ecx", "edx");
#endif
    }
}

static void hook_banner_textures(HANDLE process, const MODULEINFO &module_info)
{
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
#pragma pack(push, 1)

        struct call_replacement {
            uint8_t load_opcode;
            uint32_t addr;
            uint8_t call_opcode;
            uint8_t reg_index;
        };

#pragma pack(pop)

        struct call_replacement d3_package_load_call_replacement {
            .load_opcode = 0xB8, .addr = reinterpret_cast<uint32_t>(banner_load_hook),
            .call_opcode = 0xFF, .reg_index = 0xD0,
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

extern "C" DLL_EXPORT bool __cdecl ultimate_dll_entry_init(char *sid_code, void *app_config)
{
    DWORD pid;
    HANDLE process;
    HMODULE avs2_core_handle, jubeat_handle, music_db_handle, pkfs_handle;
    MODULEINFO jubeat_info, music_db_info, pkfs_info;

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
    if (!GetModuleInformation(process, pkfs_handle, &pkfs_info, sizeof(pkfs_info))) {
        log_fatal("GetModuleInformation(\"pkfs.dll\") failed: 0x%08lx", GetLastError());
    }

    log_info(
        "mdb_arr_patch: 0x%x score_arr_patch: 0x%x end_score_arr_patch: 0x%x",
        mdb_arr_patch,
        score_arr_patch,
        end_score_arr_patch);

    // broken for now, no longer crashes on 2020120801 but does not skip correctly
    //do_patch(process, jubeat_info, tutorial_skip);
    //do_patch(process, jubeat_info, tutorial_skip2);
    do_patch(process, jubeat_info, select_timer_freeze);
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
    do_patch(process, music_db_info, song_unlock_patch);
    /*
    do_patch(process, jubeat_info, smc_mm_text_ja);
    do_patch(process, jubeat_info, smc_mm_text_ko);
    do_patch(process, jubeat_info, smc_mm_hierarchy_ja);
    do_patch(process, jubeat_info, smc_mm_hierarchy_ko);
    */

    hook_music_db(process, jubeat_handle, music_db_handle);
    hook_pkfs_fs_open(process, pkfs_handle, pkfs_info);
    hook_banner_textures(process, jubeat_info);

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
    return TRUE;
}
