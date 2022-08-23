#define LOG_MODULE "extend"

#include <vector>

// clang-format off
#include <windows.h>
#include <psapi.h>
// clang-format on

#include <stdint.h>

#include "imports/avs2-core/avs.h"
#include "imports/gftools.h"
#include "imports/jubeat.h"

#include "pattern/pattern.h"

#include "pe/iat.h"

#include "util/defs.h"
#include "util/log.h"
#include "util/mem.h"
#include "util/patch.h"
#include "util/x86.h"

#include "music_db.h"
#include "pkfs.h"

#include "../common/bnr_hook.h"

// clang-format off
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

static std::vector<const char*> BNR_TEXTURES = {
    "L44FO_BNR_J_EX_001",
    "L44FO_BNR_J_EX_002",
    "L44FO_BNR_J_EX_003",
    "L44FO_BNR_J_EX_004",
    "L44FO_BNR_J_EX_005",
    "L44FO_BNR_J_99_999",
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

    log_info("avs_strlcpy entry = %p", avs_strlcpy_entry_ptr);
    log_info("avs_strlen entry = %p", avs_strlen_entry_ptr);
    log_info("avs_snprintf entry = %p", avs_snprintf_entry_ptr);

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

extern "C" DLL_EXPORT bool __cdecl extend_dll_entry_init(char *sid_code, void *app_config)
{
    DWORD pid;
    HANDLE process;
    HMODULE avs2_core_handle, jubeat_handle, music_db_handle, pkfs_handle;
    MODULEINFO jubeat_info, music_db_info;

    log_to_external(log_body_misc, log_body_info, log_body_warning, log_body_fatal);

    log_info("jubeat extend hook by Felix v" OMNIMIX_VERSION " (Build " __DATE__ " " __TIME__ ")");

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
    log_info("jubeat.dll = %p", reinterpret_cast<void *>(jubeat_handle));
    log_info("music_db.dll = %p", reinterpret_cast<void *>(music_db_handle));
    log_info("sid_code = %s", sid_code);
#endif

    if (!GetModuleInformation(process, jubeat_handle, &jubeat_info, sizeof(jubeat_info))) {
        log_fatal("GetModuleInformation(\"jubeat.dll\") failed: 0x%08lx", GetLastError());
    }
    if (!GetModuleInformation(process, music_db_handle, &music_db_info, sizeof(music_db_info))) {
        log_fatal("GetModuleInformation(\"music_db.dll\") failed: 0x%08lx", GetLastError());
    }

    do_patch(process, jubeat_info, packlist_pluslist);
    do_patch(process, music_db_info, music_db_limit_1);
    do_patch(process, music_db_info, music_db_limit_2_old);
    do_patch(process, music_db_info, music_db_limit_2_new);
    do_patch(process, music_db_info, music_db_limit_3);
    do_patch(process, music_db_info, music_db_limit_4);
    do_patch(process, music_db_info, music_plus_patch);

    hook_music_db(process, jubeat_handle);
    hook_pkfs_fs_open(process, pkfs_handle);
    bnr_hook_init(&jubeat_info, BNR_TEXTURES);

    CloseHandle(process);

    // Prevent the game from overriding the `rev` field
    SetEnvironmentVariableA("MB_MODEL", "----");

    // Call original
    bool ret = dll_entry_init(sid_code, app_config);

    // Set `rev` to indicate extend
    sid_code[5] = 'Y';

    return ret;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    (void) hinstDLL;
    (void) fdwReason;
    (void) lpvReserved;

    return TRUE;
}
