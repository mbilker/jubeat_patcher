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

#include "pattern/pattern.h"

#include "pe/apphook.h"
#include "pe/iat.h"

#include "util/defs.h"
#include "util/log.h"
#include "util/mem.h"
#include "util/str.h"
#include "util/x86.h"

#include "categories.h"
#include "category_hooks.h"
#include "music_db.h"
#include "pkfs.h"
#include "sound.h"

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

// it has one for each of the 3 difficulties
static uint8_t score_stack_replacer[MAX_SONGS * 3][20];
static const uintptr_t score_arr_patch = reinterpret_cast<uintptr_t>(score_stack_replacer);
// the asm to replace needs the end of the array
static const uintptr_t end_score_arr_patch =
    reinterpret_cast<uintptr_t>(&score_stack_replacer[MAX_SONGS]);

// build_and_sort_song_list's output buffer, need to bump it as otherwise "All
// Songs" category with Level grouping will truncate
static uint8_t sort_output_buf[MAX_SONGS * 3][0x20];
static const uintptr_t sort_output_buf_addr = reinterpret_cast<uintptr_t>(sort_output_buf);
static const uintptr_t sort_output_buf_plus4 = sort_output_buf_addr + 4;
static const size_t sort_output_buf_size = sizeof(sort_output_buf);
static const size_t sort_output_buf_capacity = MAX_SONGS * 3;

// sub_101a8a00 consumes one entry per sort-output entry plus up to two padding
// entries per group change (3-column layout) plus ~3 header entries. Original
// allocs 9 extra, just copy that.
static const size_t ui_state_buf_entry_count = MAX_SONGS * 3 + 9;
static uint8_t ui_state_buf[ui_state_buf_entry_count][0x74];
static const uintptr_t ui_state_buf_addr = reinterpret_cast<uintptr_t>(ui_state_buf);
static const uintptr_t ui_state_buf_plus8 = ui_state_buf_addr + 8;
// Various iterators we need to rewrite
static const uintptr_t ui_state_buf_plus14 = ui_state_buf_addr + 0x14;
static const uintptr_t ui_state_buf_plus74 = ui_state_buf_addr + 0x74;
static const uintptr_t ui_state_buf_plus7c = ui_state_buf_addr + 0x7c;
static const uintptr_t ui_state_buf_end_addr = ui_state_buf_addr + sizeof(ui_state_buf);
// Replaces the 0x1827 iter-count cap in sub_101a8a00.
static const uint32_t ui_state_buf_iter_cap = ui_state_buf_entry_count - 1;

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

// mov eax,[ebp+0x18]; test eax,eax; je near; push 0x30000 -> new size.
const struct patch_t mdb_array_4_0_memset_size {
    .name = "mdb 4.0 (sort_output mem_set size)",
    .pattern = {
        0x8B, 0x45, 0x18,                       // mov eax, [ebp+0x18]
        0x85, 0xC0,                             // test eax, eax
        0x0F, 0x84, 0x19, 0x02, 0x00, 0x00,     // je 0x101a515a (near, rel-offset: stable)
        0x68, 0x00, 0x00, 0x03, 0x00,           // push 0x30000
    },
    .data = { U32_TO_CONST_BYTES_LE(sort_output_buf_size) },
    .data_offset = 12,
};

// xor bl, 1; movzx eax, bl; mov edx, edi; push 0x1800 -> new capacity.
const struct patch_t mdb_array_4_1_capacity {
    .name = "mdb 4.1 (sort_output capacity)",
    .pattern = {
        0x80, 0xF3, 0x01,           // xor bl, 0x1
        0x0F, 0xB6, 0xC3,           // movzx eax, bl
        0x8B, 0xD7,                 // mov edx, edi
        0x68, 0x00, 0x18, 0x00, 0x00, // push 0x1800
    },
    .data = { U32_TO_CONST_BYTES_LE(sort_output_buf_capacity) },
    .data_offset = 9,
};

// mov [ebp-0x10], eax; test eax,eax; jle near; mov ebx, 0x207d5de4 ->
// mov ebx, sort_output_buf+4.  (The other 0x207d5de4 ref lives inside
// build_song_list_data207a5de0 and targets a *different* buffer, so we have
// to disambiguate with surrounding context.)
const struct patch_t mdb_array_4_2_iter {
    .name = "mdb 4.2 (sort_output iter base+4)",
    .pattern = {
        0x89, 0x45, 0xF0,                       // mov [ebp-0x10], eax
        0x85, 0xC0,                             // test eax, eax
        0x0F, 0x8E, 0xD6, 0x00, 0x00, 0x00,     // jle near (rel-offset: stable)
        0xBB,                                    // mov ebx, imm32 (imm32 is relocated)
    },
    .data = { U32_TO_CONST_BYTES_LE(sort_output_buf_plus4) },
    .data_offset = 12,
};

const struct patch_t ui_state_iter_cap_1 {
    .name = "ui_state iter cap (cmp; jg near)",
    .pattern = {
        0x81, 0xFE, 0x27, 0x18, 0x00, 0x00,     // cmp esi, 0x1827
        0x0F, 0x8F, 0xF7, 0x00, 0x00, 0x00,     // jg near 0x101a8bf3
    },
    .data = { U32_TO_CONST_BYTES_LE(ui_state_buf_iter_cap) },
    .data_offset = 2,
};
const struct patch_t ui_state_iter_cap_2 {
    .name = "ui_state iter cap (cmp; jg short)",
    .pattern = {
        0x81, 0xFE, 0x27, 0x18, 0x00, 0x00,     // cmp esi, 0x1827
        0x7F, 0x78,                             // jg short 0x101a8bf3
    },
    .data = { U32_TO_CONST_BYTES_LE(ui_state_buf_iter_cap) },
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

// ultimate categories/folders
static std::vector<const char *> EXTRA_CATEGORIES {
    "L44FO_SMC_MM_TEXT_UL",
};

// ultimate hierarchies
static std::vector<const char *> EXTRA_HIERARCHIES {
    "L44FO_SMC_MM_HIERARCHY_UL",
};

// clang-format on

// Replace every absolute-address imm32 operand referencing `stock_imm` in
// .text with `new_imm`. Driven off the module's base-relocation directory:
// every legitimate imm32 pointer into the stock module *must* have a
// HIGHLOW entry there, so we only ever inspect genuine pointer operands.
// This eliminates the risk that a byte scan collides with an unrelated
// instruction/operand whose bytes happen to match `stock_imm + delta` after
// loader fixup.
//
// NOTE: the Windows loader overwrites the in-memory OptionalHeader.ImageBase
// with the actual load address, so we can't recover the preferred base from
// the running module. Caller passes it in explicitly.
static void patch_imm32_all(
    HANDLE process,
    const MODULEINFO &module_info,
    uint32_t stock_imm,
    uintptr_t new_imm,
    const char *name,
    int expected_count,
    // jubeat.dll's base is 0x10000000. PE loader writes the actual load address
    // over OptionalHeader.ImageBase in memory, so we can't recover it at
    // runtime (well, could just open the DLL but meh)
    uintptr_t preferred_base = 0x10000000)
{
    uintptr_t actual = reinterpret_cast<uintptr_t>(module_info.lpBaseOfDll);
    uintptr_t delta = actual - preferred_base;
    uint32_t needle_imm = stock_imm + static_cast<uint32_t>(delta);

    auto dos = reinterpret_cast<PIMAGE_DOS_HEADER>(module_info.lpBaseOfDll);
    auto nt = reinterpret_cast<PIMAGE_NT_HEADERS>(reinterpret_cast<uint8_t *>(dos) + dos->e_lfanew);
    uint8_t *base = reinterpret_cast<uint8_t *>(dos);

    uint8_t *text_start = nullptr;
    size_t text_size = 0;
    PIMAGE_SECTION_HEADER section_header = IMAGE_FIRST_SECTION(nt);
    for (size_t i = 0; i < nt->FileHeader.NumberOfSections; section_header++, i++) {
        auto sname = reinterpret_cast<const char *>(section_header->Name);
        if (str_eq(".text", sname)) {
            text_start = base + section_header->VirtualAddress;
            text_size = section_header->Misc.VirtualSize;
            break;
        }
    }
    if (!text_start) {
        log_warning("%s: .text not found, aborting", name);
        return;
    }

    const IMAGE_DATA_DIRECTORY &reloc_dir =
        nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];
    if (reloc_dir.Size == 0 || reloc_dir.VirtualAddress == 0) {
        log_warning("%s: no base-reloc directory, aborting", name);
        return;
    }

    int count = 0;
    uint8_t *reloc_cur = base + reloc_dir.VirtualAddress;
    uint8_t *reloc_end = reloc_cur + reloc_dir.Size;

    while (reloc_cur + sizeof(IMAGE_BASE_RELOCATION) <= reloc_end) {
        auto block = reinterpret_cast<PIMAGE_BASE_RELOCATION>(reloc_cur);
        if (block->SizeOfBlock < sizeof(IMAGE_BASE_RELOCATION) ||
            reloc_cur + block->SizeOfBlock > reloc_end)
        {
            break;
        }
        auto entries =
            reinterpret_cast<const uint16_t *>(reloc_cur + sizeof(IMAGE_BASE_RELOCATION));
        size_t entry_count =
            (block->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(uint16_t);

        for (size_t i = 0; i < entry_count; i++) {
            uint16_t e = entries[i];
            uint16_t type = static_cast<uint16_t>(e >> 12);
            uint16_t offset = static_cast<uint16_t>(e & 0x0fff);
            if (type != IMAGE_REL_BASED_HIGHLOW) {
                continue;
            }
            uint8_t *site = base + block->VirtualAddress + offset;
            if (site < text_start || site + 4 > text_start + text_size) {
                continue;
            }
            uint32_t dword = *reinterpret_cast<const uint32_t *>(site);
            if (dword == needle_imm) {
                memory_write_ptr(process, site, new_imm);
                count++;
            }
        }

        reloc_cur += block->SizeOfBlock;
    }

    log_info(
        "%s: patched %d site(s) (stock=0x%x runtime=0x%x -> 0x%x)",
        name,
        count,
        stock_imm,
        needle_imm,
        static_cast<uint32_t>(new_imm));
    if (count != expected_count) {
        log_warning("%s: site count mismatch: expected %d, got %d", name, expected_count, count);
    }
}

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

bool __cdecl ultimate_dll_entry_init(char *sid_code, void *app_config)
{
    DWORD pid;
    HANDLE process;
    HMODULE avs2_core_handle, jubeat_handle, music_db_handle, pkfs_handle;
    MODULEINFO jubeat_info, music_db_info;

    log_info("--- Begin ultimix dll_entry_init ---");

    pid = GetCurrentProcessId();
    process = OpenProcess(
        PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE,
        false,
        pid);

    if ((avs2_core_handle = GetModuleHandleA("avs2-core.dll")) == nullptr) {
        log_fatal("GetModuleHandle(\"avs2-core.dll\") failed: 0x%08lx", GetLastError());
    }
    if ((jubeat_handle = app_hook_get_dll_handle()) == nullptr) {
        log_fatal("\"jubeat.dll\" handle is NULL");
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

    // mdb_array_4_*: expand the sort-output / scroll-panel UI buffers so all
    // songs (not just the first 2048 in iteration order) make it through the
    // filter pipeline. See comments on sort_output_buf / ui_state_buf above.
    //
    // These three specific patches must run BEFORE patch_imm32_all below,
    // because the imm32 helpers rewrite the 0x207d5de0 byte sequence that
    // these specific patches use to disambiguate themselves.
    do_patch(process, jubeat_info, mdb_array_4_0_memset_size);
    do_patch(process, jubeat_info, mdb_array_4_1_capacity);
    do_patch(process, jubeat_info, mdb_array_4_2_iter);

    // Wholesale redirect of every imm32 reference to the stock buffers.
    // Safe because each of these 4-byte values only occurs in .text as an
    // imm32 operand (we verified file-wide occurrence counts). Expected
    // site counts are baked in so a build/version drift surfaces as a
    // log_warning rather than silent under-patching. Totals: 28 sites.
    patch_imm32_all(
        process,
        jubeat_info,
        0x207d5de0,
        sort_output_buf_addr,
        "data_207d5de0 -> sort_output_buf",
        4);
    patch_imm32_all(
        process, jubeat_info, 0x10fc8170, ui_state_buf_addr, "data_10fc8170 -> ui_state_buf", 11);
    patch_imm32_all(
        process, jubeat_info, 0x10fc8178, ui_state_buf_plus8, "data_10fc8178 -> ui_state_buf+8", 4);
    patch_imm32_all(
        process,
        jubeat_info,
        0x10fc8184,
        ui_state_buf_plus14,
        "data_10fc8184 -> ui_state_buf+0x14",
        1);
    patch_imm32_all(
        process,
        jubeat_info,
        0x10fc81e4,
        ui_state_buf_plus74,
        "data_10fc81e4 -> ui_state_buf+0x74",
        2);
    patch_imm32_all(
        process,
        jubeat_info,
        0x10fc81ec,
        ui_state_buf_plus7c,
        "data_10fc81ec -> ui_state_buf+0x7c",
        1);
    patch_imm32_all(
        process, jubeat_info, 0x1107731c, ui_state_buf_end_addr, "ui_state end pointer", 3);

    do_patch(process, jubeat_info, ui_state_iter_cap_1);
    do_patch(process, jubeat_info, ui_state_iter_cap_2);
    /*
    do_patch(process, jubeat_info, smc_mm_text_ja);
    do_patch(process, jubeat_info, smc_mm_text_ko);
    do_patch(process, jubeat_info, smc_mm_hierarchy_ja);
    do_patch(process, jubeat_info, smc_mm_hierarchy_ko);
    */

    // hook_sound(process);
    hook_music_db(process, jubeat_handle, music_db_handle);
    hook_pkfs_fs_open(process, pkfs_handle);
    bnr_hook_init(jubeat_info);
    bnr_hook_add_paths("L44_BNR_BIG_ID99999999", BNR_TEXTURES);
    bnr_hook_add_paths("L44_TM_BANNER", EXTRA_MARKERS);
    bnr_hook_add_paths("L44FO_SMC_MM_TEXT_JA", EXTRA_CATEGORIES);
    bnr_hook_add_paths("L44FO_SMC_MM_HIERARCHY_JA", EXTRA_HIERARCHIES);
    festo_apply_common_patches(process, jubeat_handle, jubeat_info, music_db_info);

    // category stuff
    category_hooks_add_category_definitions(extra_category_hierarchy);
    category_hooks_add_category_layouts(extra_category_layout);
    category_hooks_add_group_textures(extra_group_textures);
    category_hooks_add_grouping_hook_fn(category_group_fn_alphabet);
    category_hooks_add_grouping_hook_fn(category_group_fn_genre_custom);
    category_hooks_add_grouping_hook_fn(category_group_fn_version_custom);
    category_hooks_init(process, jubeat_info);

    MH_EnableHook(MH_ALL_HOOKS);

    CloseHandle(process);

    // Prevent the game from overriding the `rev` field
    SetEnvironmentVariableA("MB_MODEL", "----");

    // Call original
    bool ret = app_hook_invoke_init(sid_code, app_config);

    // Set `rev` to indicate ultimate
    sid_code[5] = 'U';

    log_info("---  End  ultimix dll_entry_init ---");

    return ret;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    (void) hinstDLL;
    (void) lpvReserved;

    if (fdwReason != DLL_PROCESS_ATTACH) {
        return TRUE;
    }

    log_to_external(log_body_misc, log_body_info, log_body_warning, log_body_fatal);

    log_info(
        "jubeat ultimate hook by Felix, Cannu & mon v" OMNIMIX_VERSION " (Build " __DATE__
        " " __TIME__ ")");

    MH_Initialize();
    app_hook_init(ultimate_dll_entry_init, NULL);
    MH_EnableHook(MH_ALL_HOOKS);

    return TRUE;
}
