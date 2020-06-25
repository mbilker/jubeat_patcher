#define LOG_MODULE "extend"

#include <vector>

#include <windows.h>
#include <dbghelp.h>
#include <psapi.h>

#include <stdint.h>

#include "imports/avs2-core/avs.h"
#include "imports/gftools.h"
#include "imports/jubeat.h"

#include "pattern/pattern.h"

#include "util/log.h"

#include "music_db.h"
#include "pkfs.h"

static void *D3_PACKAGE_LOAD = nullptr;
static const char *BNR_TEXTURES[] = {
    "L44FO_BNR_J_EX_001",
    "L44FO_BNR_J_EX_002",
    "L44FO_BNR_J_EX_003",
    "L44FO_BNR_J_EX_004",
    "L44FO_BNR_J_EX_005",
    "L44FO_BNR_J_99_999",
};

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
    .name = "pluslist patch",
    .pattern = { 'p', 'a', 'c', 'k', 'l', 'i', 's', 't' },
    .data = { 'p', 'l', 'u', 's' },
    .data_offset = 0,
};

const struct patch_t music_db_limit_1 {
    .name = "music_db limit patch 1",
    .pattern = { 0x00, 0x00, 0x20, 0x00, 0x57, 0xFF, 0x15 },
    .data = { 0x40 },
    .data_offset = 2,
};
const struct patch_t music_db_limit_2_old {
    .name = "music_db limit patch 2 (old)",
    .pattern = { 0x00, 0x00, 0x20, 0x00, 0x8B, 0xF8, 0x57 },
    .data = { 0x40 },
    .data_offset = 2,
};
const struct patch_t music_db_limit_2_new {
    .name = "music_db limit patch 2 (new)",
    .pattern = { 0x00, 0x00, 0x20, 0x00, 0x53, 0x6A, 0x01 },
    .data = { 0x40 },
    .data_offset = 2,
};
const struct patch_t music_db_limit_3 {
    .name = "music_db limit patch 3",
    .pattern = { 0x00, 0x00, 0x20, 0x00, 0x6A, 0x00, 0xFF },
    .data = { 0x40 },
    .data_offset = 2,
};
const struct patch_t music_db_limit_4 {
    .name = "music_db limit patch 4",
    .pattern = { 0x00, 0x00, 0x20, 0x00, 0x50, 0x6A, 0x17 },
    .data = { 0x40 },
    .data_offset = 2,
};

const struct patch_t music_plus_patch {
    .name = "music_plus patch",
    .pattern = { 'm', 'u', 's', 'i', 'c', '_', 'i', 'n', 'f', 'o', '.', 'x', 'm', 'l' },
    .data = { 'p', 'l', 'u', 's' },
    .data_offset = 6,
};

const struct patch_t song_unlock_patch {
    .name = "song unlock",
    .pattern = { 0xC4, 0x04, 0x84, 0xC0, 0x74, 0x09 },
    .data = { 0x90, 0x90 },
    .data_offset = 4,
};

static DWORD memory_make_rw(HANDLE process, void *target, size_t data_size) {
    DWORD old_protect;

    if (!VirtualProtectEx(process, target, data_size, PAGE_EXECUTE_READWRITE, &old_protect)) {
        log_fatal("VirtualProtectEx (rwx) failed: 0x%08lx", GetLastError());
    }

    return old_protect;
}

static void memory_restore_old(HANDLE process, void *target, size_t data_size, DWORD old_protect) {
    if (!VirtualProtectEx(process, target, data_size, old_protect, &old_protect)) {
        log_fatal("VirtualProtectEx (old) failed: 0x%08lx", GetLastError());
    }
}

static void do_write(HANDLE process, void *target, const void *data, size_t data_size) {
    DWORD old_protect;

    old_protect = memory_make_rw(process, target, data_size);

    WriteProcessMemory(process, target, data, data_size, nullptr);
    FlushInstructionCache(process, target, data_size);

    memory_restore_old(process, target, data_size, old_protect);
}

static void do_write(HANDLE process, void *target, const std::vector<uint8_t> &data) {
    do_write(process, target, data.data(), data.size());
}

static void do_set(HANDLE process, void *target, uint8_t data_value, size_t data_size) {
    void *buf;

    buf = HeapAlloc(GetProcessHeap(), 0, data_size);

    if (buf == nullptr) {
        log_fatal("HeapAlloc(%u) failed: 0x%08lx", data_size, GetLastError());
    }

    // Fill the allocated buffer with the value desired
    memset(buf, data_value, data_size);

    do_write(process, target, buf, data_size);

    HeapFree(GetProcessHeap(), 0, buf);
}

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

    if (patch.pattern_mask != nullptr) {
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

    if (addr != nullptr) {
#ifdef VERBOSE
        hex_data = to_hex(addr, patch.pattern.size());
        log_info("data: %s", hex_data);
        free(hex_data);
#endif

        target = &addr[patch.data_offset];

        do_write(process, target, patch.data);

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

static void hook_iat(
        HANDLE process,
        HMODULE module,
        const char *target_module_name,
        void *func_ptr,
        void *target_func_ptr)
{
    IMAGE_IMPORT_DESCRIPTOR *import_descriptor;
    unsigned long size;
    const char *module_name;
    IMAGE_THUNK_DATA *thunk_data;
    void *target;

    import_descriptor = reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR *>(ImageDirectoryEntryToData(
            module,
            true,
            IMAGE_DIRECTORY_ENTRY_IMPORT,
            &size));

    if (import_descriptor == nullptr) {
        log_warning("failed to get import descriptor for module: 0x%08lx", GetLastError());
        return;
    }
    if (size == 0) {
        log_warning("no imports for module %p", module);
        return;
    }

    while (import_descriptor->Name != 0) {
        module_name = reinterpret_cast<const char *>(
                reinterpret_cast<uintptr_t>(module) + static_cast<uintptr_t>(import_descriptor->Name));

        if (_stricmp(module_name, target_module_name) == 0) {
            break;
        }

        import_descriptor++;
    }

    if (import_descriptor->Name == 0) {
        log_warning("failed to find import descriptor for '%s'", target_module_name);
        return;
    }

    log_misc("found import descriptor for '%s' at %p", target_module_name, import_descriptor);

    thunk_data = reinterpret_cast<IMAGE_THUNK_DATA *>(
            reinterpret_cast<uintptr_t>(module) + static_cast<uintptr_t>(import_descriptor->FirstThunk));

    while (thunk_data->u1.Function != 0) {
        if (reinterpret_cast<void *>(thunk_data->u1.Function) == func_ptr) {
            target = &thunk_data->u1.Function;

            do_write(process, target, &target_func_ptr, sizeof(thunk_data->u1.Function));
            log_misc("patched %p in '%s' with %p", target, target_module_name, target_func_ptr);

            break;
        }

        thunk_data++;
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

    {
        // Compute address to the `E8 xx xx xx xx` (relative jump) instruction
        uintptr_t jump_addr = reinterpret_cast<uintptr_t>(d3_package_load_call_addr) + 2;

        // Load the offset from the relative jump instruction
        uint32_t offset = *reinterpret_cast<uint32_t *>(jump_addr + 1);

        // Compute and save the absolute address of the function for later
        D3_PACKAGE_LOAD = reinterpret_cast<void *>(jump_addr + 5 + offset);
    }

    struct call_replacement {
        uint8_t load_opcode;
        uint32_t addr;
        uint8_t call_opcode;
        uint8_t reg_index;
    } __attribute__((packed));

    {
        struct call_replacement d3_package_load_call_replacement {
            .load_opcode = 0xB8,
            .addr = reinterpret_cast<uint32_t>(banner_load_hook),
            .call_opcode = 0xFF,
            .reg_index = 0xD0,
        };
        static_assert(sizeof(d3_package_load_call_replacement) == sizeof(d3_package_load_pattern));

        do_write(
                process,
                d3_package_load_call_addr,
                &d3_package_load_call_replacement,
                sizeof(d3_package_load_call_replacement));
    }

    // `nop` out the loop increment and jump part (adding one to include the jump target)
    do_set(process, loop_jnz_addr, 0x90, sizeof(loop_jnz_pattern) + 1);
}

extern "C" bool __declspec(dllexport) dll_entry_init(char *sid_code, void *app_config) {
    DWORD pid;
    HANDLE process;
    HMODULE jubeat_handle, music_db_handle, pkfs_handle;
    MODULEINFO jubeat_info, music_db_info;
    uint8_t *jubeat;
#ifdef VERBOSE
    uint8_t *music_db;
#endif
    uint8_t *pkfs;
    FARPROC music_db_get_sequence_filename;
    FARPROC music_db_get_sound_filename;

    log_to_external(log_body_misc, log_body_info, log_body_warning, log_body_fatal);

    log_info("jubeat extend hook by Felix v" OMNIMIX_VERSION " (Build " __DATE__ " " __TIME__ ")");

    pid = GetCurrentProcessId();
    process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE, false, pid);

    if ((jubeat_handle = GetModuleHandleA("jubeat.dll")) == nullptr) {
        log_fatal("GetModuleHandle(\"jubeat.dll\") failed: 0x%08lx", GetLastError());
    }
    if ((music_db_handle = GetModuleHandleA("music_db.dll")) == nullptr) {
        log_fatal("GetModuleHandle(\"music_db.dll\") failed: 0x%08lx", GetLastError());
    }
    if ((pkfs_handle = GetModuleHandleA("pkfs.dll")) == nullptr) {
        log_fatal("GetModuleHandle(\"pkfs.dll\") failed: 0x%08lx", GetLastError());
    }

    jubeat = reinterpret_cast<uint8_t *>(jubeat_handle);
#ifdef VERBOSE
    music_db = reinterpret_cast<uint8_t *>(music_db_handle);
#endif
    pkfs = reinterpret_cast<uint8_t *>(pkfs_handle);

#ifdef VERBOSE
    log_info("jubeat.dll = %p, music_db.dll = %p", jubeat, music_db);
    log_info("sid_code = %s", sid_code);
#endif

    if (!GetModuleInformation(process, jubeat_handle, &jubeat_info, sizeof(jubeat_info))) {
        log_fatal("GetModuleInformation(\"jubeat.dll\") failed: 0x%08lx", GetLastError());
    }
    if (!GetModuleInformation(process, music_db_handle, &music_db_info, sizeof(music_db_info))) {
        log_fatal("GetModuleInformation(\"music_db.dll\") failed: 0x%08lx", GetLastError());
    }

    do_patch(process, jubeat_info, tutorial_skip);
    do_patch(process, jubeat_info, select_timer_freeze);
    do_patch(process, jubeat_info, packlist_pluslist);
    do_patch(process, music_db_info, music_db_limit_1);
    do_patch(process, music_db_info, music_db_limit_2_old);
    do_patch(process, music_db_info, music_db_limit_2_new);
    do_patch(process, music_db_info, music_db_limit_3);
    do_patch(process, music_db_info, music_db_limit_4);
    do_patch(process, music_db_info, music_plus_patch);
    do_patch(process, music_db_info, song_unlock_patch);

    if ((music_db_get_sequence_filename = GetProcAddress(music_db_handle, "music_db_get_sequence_filename")) == nullptr) {
        log_fatal("GetProcAddress(\"music_db.dll\", \"music_db_get_sequence_filename\") failed: 0x%08lx", GetLastError());
    }
    if ((music_db_get_sound_filename = GetProcAddress(music_db_handle, "music_db_get_sound_filename")) == nullptr) {
        log_fatal("GetProcAddress(\"music_db.dll\", \"music_db_get_sound_filename\") failed: 0x%08lx", GetLastError());
    }

    hook_iat(
            process,
            jubeat_handle,
            "music_db.dll",
            reinterpret_cast<void *>(music_db_get_sequence_filename),
            reinterpret_cast<void *>(music_db_get_sequence_filename_hook));
    hook_iat(
            process,
            jubeat_handle,
            "music_db.dll",
            reinterpret_cast<void *>(music_db_get_sound_filename),
            reinterpret_cast<void *>(music_db_get_sound_filename_hook));

    // TODO(felix): make these patches version agnostic by using patterns

    struct addr_relative_replacement {
        uint8_t opcode;
        uint32_t addr_offset;
        uint8_t nop;
    } __attribute__((packed));

    {
        uint32_t call = reinterpret_cast<uintptr_t>(pkfs_avs_strlcpy) -
                reinterpret_cast<uintptr_t>(pkfs + 0x1935) -
                5;

        struct addr_relative_replacement strlcpy_patch {
            .opcode = 0xE8u,
            .addr_offset = call,
            .nop = 0x90u,
        };
        do_write(process, &pkfs[0x1935], &strlcpy_patch, sizeof(strlcpy_patch));
    }
    {
        uint32_t call = reinterpret_cast<uintptr_t>(pkfs_avs_strlen) -
                reinterpret_cast<uintptr_t>(pkfs + 0x1959) -
                5;

        struct addr_relative_replacement strlen_patch {
            .opcode = 0xE8u,
            .addr_offset = call,
            .nop = 0x90u,
        };
        do_write(process, &pkfs[0x1959], &strlen_patch, sizeof(strlen_patch));
    }
    {
        struct addr_relative_replacement snprintf_patch {
            .opcode = 0xBEu,
            .addr_offset = reinterpret_cast<uint32_t>(pkfs_avs_snprintf),
            .nop = 0x90u,
        };
        do_write(process, &pkfs[0x19F3], &snprintf_patch, sizeof(snprintf_patch));
    }

    hook_banner_textures(process, jubeat_info);

    /*
#ifdef VERBOSE
    for (size_t i = 0; i < 18; i++) {
        char *hex_data = to_hex(&jubeat[0xC6DD + i * 8], 8);
        log_info("data: %s", hex_data);
        free(hex_data);
    }
    for (size_t i = 0; i < 5; i++) {
        char *hex_data = to_hex(&jubeat[0xC76D + i * 11], 11);
        log_info("data: %s", hex_data);
        free(hex_data);
    }
    {
        char *hex_data = to_hex(&jubeat[0xC7ED], 5);
        log_info("data: %s", hex_data);
        free(hex_data);
    }
#endif

    // Overwrite the pointers to point into our texture list
    for (size_t i = 0; i < std::min(std::size(BNR_TEXTURES), 18u); i++) {
        do_write(process, &jubeat[0xC6DD + i * 8 + 4], &BNR_TEXTURES[i], 4);
    }
    // Overwrite the rest of the list with null pointers
    for (size_t i = std::size(BNR_TEXTURES); i < 18; i++) {
        do_write(process, &jubeat[0xC6DD + i * 8 + 4], (const uint8_t[]) { 0, 0, 0, 0 }, 4);
    }
    // Overwrite the other part of the list that uses a different store instruction
    for (size_t i = 0; i < 5; i++) {
        if (i + 18 < std::size(BNR_TEXTURES)) {
            do_write(process, &jubeat[0xC76D + i * 11 + 7], &BNR_TEXTURES[i + 18], 4);
        } else {
            do_write(process, &jubeat[0xC76D + i * 11 + 7], (const uint8_t[]) { 0, 0, 0, 0 }, 4);
        }
    }

    // Write the loop starting value
    do_write(process, &jubeat[0xC7ED + 1], &BNR_TEXTURES[0], 4);
    */

    CloseHandle(process);

    bool ret = jb_dll_entry_init(sid_code, app_config);

    // Set `rev` to indicate extend
    sid_code[5] = 'Y';

    return ret;

}

extern "C" bool __declspec(dllexport) dll_entry_main(void) {
    return jb_dll_entry_main();
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    return TRUE;
}
