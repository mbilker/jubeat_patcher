#define LOG_MODULE "omnimix"

#include <windows.h>
#include <psapi.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "imports/avs2-core/avs.h"
#include "imports/jubeat.h"

#include "pattern/pattern.h"

#include "util/log.h"
#include "util/mem.h"

struct patch_t {
    const char *name;
    const uint8_t *pattern;
    const bool *pattern_mask;
    size_t pattern_size;
    const uint8_t *data;
    size_t data_size;
    size_t data_offset;
};

// jubeat 2018081401:
// 0xD0A67 offset in address space
const uint8_t tutorial_skip_pattern[] = { 0x3D, 0x21, 0x00, 0x00, 0x80, 0x75, 0x4A, 0x57, 0x68, 0x00, 0x00, 0x60 };
const uint8_t tutorial_skip_data[] = { 0xE9, 0x90, 0x00, 0x00, 0x00 };

// jubeat 2018081401:
// 0xA6499 offset in address space
const uint8_t select_timer_freeze_pattern[] = { 0x01, 0x00, 0x84, 0xC0, 0x75, 0x21, 0x38, 0x05 };
const uint8_t select_timer_freeze_data[] = { 0xEB };

// jubeat 2018081401:
// 0x1F9F24 in address space
const uint8_t packlist_omnilist_pattern[] = { 'p', 'a', 'c', 'k', 'l', 'i', 's', 't' };
const uint8_t packlist_omnilist_data[] = { 'o', 'm', 'n', 'i' };

// jubeat 2018081401 music_db:
// 0x5310 in address space
const uint8_t big_bnr_pattern[] = { 0x74, 0x19, 0x8B, 0x45, 0x10, 0x8B, 0x4D, 0x0C, 0x68, 0xF4 };
const uint8_t big_bnr_data[] = { 0x75 };

// jubeat 2018081401 music_db:
// several offsets
const uint8_t music_db_limit_pattern_1[] = { 0x00, 0x00, 0x20, 0x00, 0x53, 0xFF, 0x15 };
const uint8_t music_db_limit_pattern_2[] = { 0x00, 0x00, 0x20, 0x00, 0x8B, 0xF8, 0x57 };
const uint8_t music_db_limit_pattern_3[] = { 0x00, 0x00, 0x20, 0x00, 0x6A, 0x00, 0xFF };
const uint8_t music_db_limit_pattern_4[] = { 0x00, 0x00, 0x20, 0x00, 0x50, 0x6A, 0x17 };
const uint8_t music_db_limit_data[] = { 0x40 };

// jubeat 2018081401 music_db:
// 0x152CC in address space
const uint8_t music_info_pattern[] = { 'm', 'u', 's', 'i', 'c', '_', 'i', 'n', 'f', 'o', '.', 'x', 'm', 'l' };
const uint8_t music_omni_data[] = { 'o', 'm', 'n', 'i' };

// jubeat 2018081401 music_db:
// 0x278F offset in address space
const uint8_t song_unlock_pattern[] = { 0xC4, 0x04, 0x84, 0xC0, 0x74, 0x09 };
const uint8_t song_unlock_data[] = { 0x90, 0x90 };

const struct patch_t tutorial_skip = {
    .name = "tutorial skip",
    .pattern = tutorial_skip_pattern,
    .pattern_size = sizeof(tutorial_skip_pattern),
    .data = tutorial_skip_data,
    .data_size = sizeof(tutorial_skip_data),
    .data_offset = 5,
};

const struct patch_t select_timer_freeze = {
    .name = "song select timer freeze",
    .pattern = select_timer_freeze_pattern,
    .pattern_size = sizeof(select_timer_freeze_pattern),
    .data = select_timer_freeze_data,
    .data_size = sizeof(select_timer_freeze_data),
    .data_offset = 4,
};

const struct patch_t packlist_omnilist = {
    .name = "omnilist patch",
    .pattern = packlist_omnilist_pattern,
    .pattern_size = sizeof(packlist_omnilist_pattern),
    .data = packlist_omnilist_data,
    .data_size = sizeof(packlist_omnilist_data),
    .data_offset = 0,
};

const struct patch_t big_bnr_patch = {
    .name = "big_bnr patch",
    .pattern = big_bnr_pattern,
    .pattern_size = sizeof(big_bnr_pattern),
    .data = big_bnr_data,
    .data_size = sizeof(big_bnr_data),
    .data_offset = 0,
};

const struct patch_t music_db_limit_1 = {
    .name = "music_db limit patch 1",
    .pattern = music_db_limit_pattern_1,
    .pattern_size = sizeof(music_db_limit_pattern_1),
    .data = music_db_limit_data,
    .data_size = sizeof(music_db_limit_data),
    .data_offset = 2,
};
const struct patch_t music_db_limit_2 = {
    .name = "music_db limit patch 2",
    .pattern = music_db_limit_pattern_2,
    .pattern_size = sizeof(music_db_limit_pattern_2),
    .data = music_db_limit_data,
    .data_size = sizeof(music_db_limit_data),
    .data_offset = 2,
};
const struct patch_t music_db_limit_3 = {
    .name = "music_db limit patch 3",
    .pattern = music_db_limit_pattern_3,
    .pattern_size = sizeof(music_db_limit_pattern_3),
    .data = music_db_limit_data,
    .data_size = sizeof(music_db_limit_data),
    .data_offset = 2,
};
const struct patch_t music_db_limit_4 = {
    .name = "music_db limit patch 4",
    .pattern = music_db_limit_pattern_4,
    .pattern_size = sizeof(music_db_limit_pattern_4),
    .data = music_db_limit_data,
    .data_size = sizeof(music_db_limit_data),
    .data_offset = 2,
};

const struct patch_t music_omni_patch = {
    .name = "music_omni patch",
    .pattern = music_info_pattern,
    .pattern_size = sizeof(music_info_pattern),
    .data = music_omni_data,
    .data_size = sizeof(music_omni_data),
    .data_offset = 6,
};

const struct patch_t song_unlock_patch = {
    .name = "song unlock",
    .pattern = song_unlock_pattern,
    .pattern_size = sizeof(song_unlock_pattern),
    .data = song_unlock_data,
    .data_size = sizeof(song_unlock_data),
    .data_offset = 4,
};

void do_patch(HANDLE process, const MODULEINFO *module_info, const struct patch_t *patch) {
#ifdef VERBOSE
    char *hex_data;
#endif
    uint8_t *addr, *target;

#ifdef VERBOSE
    log_info("===== %s =====", patch->name);

    hex_data = to_hex(patch->pattern, patch->pattern_size);
    log_info("pattern: %s", hex_data);
    free(hex_data);

    if (patch->pattern_mask != NULL) {
        hex_data = to_hex((const uint8_t *) patch->pattern_mask, patch->pattern_size);
        log_info("mask   : %s", hex_data);
        free(hex_data);
    }
#endif

    addr = find_pattern(module_info->lpBaseOfDll, module_info->SizeOfImage, patch->pattern, patch->pattern_mask, patch->pattern_size);

    if (addr != NULL) {
#ifdef VERBOSE
        hex_data = to_hex(addr, patch->pattern_size);
        log_info("data: %s", hex_data);
        free(hex_data);
#endif

        target = &addr[patch->data_offset];

        memory_write(process, target, patch->data, patch->data_size);

#ifdef VERBOSE
        log_info("%s applied at %p", patch->name, target);

        hex_data = to_hex(addr, patch->pattern_size);
        log_info("data: %s", hex_data);
        free(hex_data);
#endif
    } else {
        log_warning("could not find %s base address", patch->name);
    }
}

bool __declspec(dllexport) dll_entry_init(char *sid_code, void *app_config) {
    DWORD pid;
    HANDLE process;
    HMODULE jubeat_handle, music_db_handle;
    MODULEINFO jubeat_info, music_db_info;
#ifdef VERBOSE
    uint8_t *jubeat, *music_db;
#endif

    log_to_external(log_body_misc, log_body_info, log_body_warning, log_body_fatal);

    log_info("jubeat omnimix hook by Felix v" OMNIMIX_VERSION " (Build " __DATE__ " " __TIME__ ")");

    pid = GetCurrentProcessId();
    process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE, FALSE, pid);

    if ((jubeat_handle = GetModuleHandle("jubeat.dll")) == NULL) {
        log_fatal("GetModuleHandle(\"jubeat.dll\") failed: %08lx", GetLastError());
    }
    if ((music_db_handle = GetModuleHandle("music_db.dll")) == NULL) {
        log_fatal("GetModuleHandle(\"music_db.dll\") failed: %08lx", GetLastError());
    }

#ifdef VERBOSE
    jubeat = (uint8_t *) jubeat_handle;
    music_db = (uint8_t *) music_db_handle;

    log_info("jubeat.dll = %p, music_db.dll = %p", jubeat, music_db);
    log_info("sid_code = %s", sid_code);
#endif

    if (!GetModuleInformation(process, jubeat_handle, &jubeat_info, sizeof(jubeat_info))) {
        log_fatal("GetModuleInformation(\"jubeat.dll\") failed: %08lx", GetLastError());
    }
    if (!GetModuleInformation(process, music_db_handle, &music_db_info, sizeof(music_db_info))) {
        log_fatal("GetModuleInformation(\"music_db.dll\") failed: %08lx", GetLastError());
    }

    do_patch(process, &jubeat_info, &tutorial_skip);
    do_patch(process, &jubeat_info, &select_timer_freeze);
    do_patch(process, &jubeat_info, &packlist_omnilist);
    do_patch(process, &music_db_info, &big_bnr_patch);
    do_patch(process, &music_db_info, &music_db_limit_1);
    do_patch(process, &music_db_info, &music_db_limit_2);
    do_patch(process, &music_db_info, &music_db_limit_3);
    do_patch(process, &music_db_info, &music_db_limit_4);
    do_patch(process, &music_db_info, &music_omni_patch);
    do_patch(process, &music_db_info, &song_unlock_patch);

    CloseHandle(process);

    sid_code[5] = 'X';

    return jb_dll_entry_init(sid_code, app_config);
}

bool __declspec(dllexport) dll_entry_main(void) {
    return jb_dll_entry_main();
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    return TRUE;
}
