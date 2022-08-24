#define LOG_MODULE "omnimix"

// clang-format off
#include <windows.h>
#include <psapi.h>
// clang-format on

#include <stdint.h>

#include "imports/avs2-core/avs.h"
#include "imports/jubeat.h"

#include "pattern/pattern.h"

#include "util/defs.h"
#include "util/log.h"
#include "util/mem.h"

#include "../common/bnr_hook.h"
#include "../common/festo.h"

// clang-format off

const struct patch_t packlist {
    .name = "omnilist patch",
    .pattern = { 'p', 'a', 'c', 'k', 'l', 'i', 's', 't' },
    .data = { 'o', 'm', 'n', 'i' },
    .data_offset = 0,
};

const struct patch_t music_db_limit_1 {
    .name = "music_db limit patch 1",
    .pattern = { 0x00, 0x00, 0x20, 0x00, 0x57, 0xFF, 0x15 },
    .data = { 0x60 },
    .data_offset = 2,
};
const struct patch_t music_db_limit_2 {
    .name = "music_db limit patch 2",
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

const struct patch_t music_omni_patch {
    .name = "music_omni patch",
    .pattern = { 'm', 'u', 's', 'i', 'c', '_', 'i', 'n', 'f', 'o', '.', 'x', 'm', 'l' },
    .data = { 'o', 'm', 'n', 'i' },
    .data_offset = 6,
};

static const char *EXTRA_BANNERS[] = {
    "L44FO_BNR_J_OM_001",
    "L44FO_BNR_J_OM_002",
};

// clang-format on

extern "C" DLL_EXPORT bool __cdecl omnimix_dll_entry_init(char *sid_code, void *app_config)
{
    DWORD pid;
    HANDLE process;
    HMODULE jubeat_handle, music_db_handle;
    MODULEINFO jubeat_info, music_db_info;

    log_to_external(log_body_misc, log_body_info, log_body_warning, log_body_fatal);

    log_info("jubeat omnimix hook by Felix, Cannu & mon v" OMNIMIX_VERSION " (Build " __DATE__ " " __TIME__ ")");

    pid = GetCurrentProcessId();
    process = OpenProcess(
        PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE,
        FALSE,
        pid);

    if ((jubeat_handle = GetModuleHandleA("jubeat.dll")) == NULL) {
        log_fatal("GetModuleHandle(\"jubeat.dll\") failed: %08lx", GetLastError());
    }
    if ((music_db_handle = GetModuleHandleA("music_db.dll")) == NULL) {
        log_fatal("GetModuleHandle(\"music_db.dll\") failed: %08lx", GetLastError());
    }

#ifdef VERBOSE
    log_info("jubeat.dll = %p", reinterpret_cast<void *>(jubeat_handle));
    log_info("music_db.dll = %p", reinterpret_cast<void *>(music_db_handle));
    log_info("sid_code = %s", sid_code);
#endif

    if (!GetModuleInformation(process, jubeat_handle, &jubeat_info, sizeof(jubeat_info))) {
        log_fatal("GetModuleInformation(\"jubeat.dll\") failed: %08lx", GetLastError());
    }
    if (!GetModuleInformation(process, music_db_handle, &music_db_info, sizeof(music_db_info))) {
        log_fatal("GetModuleInformation(\"music_db.dll\") failed: %08lx", GetLastError());
    }

    festo_apply_common_patches(process, jubeat_handle, music_db_info);

    do_patch(process, jubeat_info, packlist);
    do_patch(process, music_db_info, music_db_limit_1);
    do_patch(process, music_db_info, music_db_limit_2);
    do_patch(process, music_db_info, music_db_limit_3);
    do_patch(process, music_db_info, music_db_limit_4);
    do_patch(process, music_db_info, music_omni_patch);

    bnr_hook_init(jubeat_info, EXTRA_BANNERS);

    CloseHandle(process);

    // Prevent the game from overriding the `rev` field
    SetEnvironmentVariableA("MB_MODEL", "----");

    // Call original
    bool ret = dll_entry_init(sid_code, app_config);

    // Set `rev` to indicate omnimix
    sid_code[5] = 'X';

    // Call original
    return ret;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    (void) hinstDLL;
    (void) fdwReason;
    (void) lpvReserved;

    return TRUE;
}
