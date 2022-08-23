#define LOG_MODULE "omnimix"

// clang-format off
#include <windows.h>
#include <psapi.h>
// clang-format on

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "imports/avs2-core/avs.h"
#include "imports/jubeat.h"

#include "pattern/pattern.h"

#include "util/defs.h"
#include "util/log.h"
#include "util/mem.h"
#include "util/patch.h"

// clang-format off

const struct patch_t packlist_omnilist = {
    .name = "omnilist patch",
    // jubeat 2018081401:
    // 0x1F9F24 in address space
    .pattern = { 'p', 'a', 'c', 'k', 'l', 'i', 's', 't' },
    .data = { 'o', 'm', 'n', 'i' },
    .data_offset = 0,
};

const struct patch_t big_bnr_patch = {
    .name = "big_bnr patch",
    // jubeat 2018081401 music_db:
    // 0x5310 in address space
    .pattern = { 0x74, 0x19, 0x8B, 0x45, 0x10, 0x8B, 0x4D, 0x0C, 0x68, 0xF4 },
    .data = { 0x75 },
    .data_offset = 0,
};

// jubeat 2018081401 music_db:
// several offsets
const struct patch_t music_db_limit_1 = {
    .name = "music_db limit patch 1",
    .pattern = { 0x00, 0x00, 0x20, 0x00, 0x53, 0xFF, 0x15 },
    .data = { 0x40 },
    .data_offset = 2,
};
const struct patch_t music_db_limit_2 = {
    .name = "music_db limit patch 2",
    .pattern = { 0x00, 0x00, 0x20, 0x00, 0x8B, 0xF8, 0x57 },
    .data = { 0x40 },
    .data_offset = 2,
};
const struct patch_t music_db_limit_3 = {
    .name = "music_db limit patch 3",
    .pattern = { 0x00, 0x00, 0x20, 0x00, 0x6A, 0x00, 0xFF },
    .data = { 0x40 },
    .data_offset = 2,
};
const struct patch_t music_db_limit_4 = {
    .name = "music_db limit patch 4",
    .pattern = { 0x00, 0x00, 0x20, 0x00, 0x50, 0x6A, 0x17 },
    .data = { 0x40 },
    .data_offset = 2,
};

const struct patch_t music_omni_patch = {
    .name = "music_omni patch",
    // jubeat 2018081401 music_db:
    // 0x152CC in address space
    .pattern = {'m', 'u', 's', 'i', 'c', '_', 'i', 'n', 'f', 'o', '.', 'x', 'm', 'l',},
    .data = { 'o', 'm', 'n', 'i' },
    .data_offset = 6,
};

const struct patch_t song_unlock_patch = {
    .name = "song unlock",
    // jubeat 2018081401 music_db:
    // 0x278F offset in address space
    .pattern = { 0xC4, 0x04, 0x84, 0xC0, 0x74, 0x09 },
    .data = { 0x90, 0x90 },
    .data_offset = 4,
};

// clang-format on

extern "C" DLL_EXPORT bool __cdecl omnimix_dll_entry_init(char *sid_code, void *app_config)
{
    DWORD pid;
    HANDLE process;
    HMODULE jubeat_handle, music_db_handle;
    MODULEINFO jubeat_info, music_db_info;

    log_to_external(log_body_misc, log_body_info, log_body_warning, log_body_fatal);

    log_info("jubeat omnimix hook by Felix v" OMNIMIX_VERSION " (Build " __DATE__ " " __TIME__ ")");

    pid = GetCurrentProcessId();
    process = OpenProcess(
        PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE,
        FALSE,
        pid);

    if ((jubeat_handle = GetModuleHandle("jubeat.dll")) == NULL) {
        log_fatal("GetModuleHandle(\"jubeat.dll\") failed: %08lx", GetLastError());
    }
    if ((music_db_handle = GetModuleHandle("music_db.dll")) == NULL) {
        log_fatal("GetModuleHandle(\"music_db.dll\") failed: %08lx", GetLastError());
    }

#ifdef VERBOSE
    log_info("jubeat.dll = %p", (void *) jubeat_handle);
    log_info("music_db.dll = %p", (void *) music_db_handle);
    log_info("sid_code = %s", sid_code);
#endif

    if (!GetModuleInformation(process, jubeat_handle, &jubeat_info, sizeof(jubeat_info))) {
        log_fatal("GetModuleInformation(\"jubeat.dll\") failed: %08lx", GetLastError());
    }
    if (!GetModuleInformation(process, music_db_handle, &music_db_info, sizeof(music_db_info))) {
        log_fatal("GetModuleInformation(\"music_db.dll\") failed: %08lx", GetLastError());
    }

    do_patch(process, jubeat_info, packlist_omnilist);
    do_patch(process, music_db_info, big_bnr_patch);
    do_patch(process, music_db_info, music_db_limit_1);
    do_patch(process, music_db_info, music_db_limit_2);
    do_patch(process, music_db_info, music_db_limit_3);
    do_patch(process, music_db_info, music_db_limit_4);
    do_patch(process, music_db_info, music_omni_patch);
    do_patch(process, music_db_info, song_unlock_patch);

    CloseHandle(process);

    sid_code[5] = 'X';

    return dll_entry_init(sid_code, app_config);
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    (void) hinstDLL;
    (void) fdwReason;
    (void) lpvReserved;

    return TRUE;
}
