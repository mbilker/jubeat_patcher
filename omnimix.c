#include <windows.h>
#include <psapi.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "capnhook/hook/table.h"
#include "util/log.h"

#define JB_DLL_ENTRY_INIT jb_dll_entry_init
#define JB_DLL_ENTRY_MAIN jb_dll_entry_main

bool __declspec(dllimport) JB_DLL_ENTRY_INIT(char *, void *);
bool __declspec(dllimport) JB_DLL_ENTRY_MAIN(void);

#define LOG_CHECK_FMT __attribute__(( format(printf, 2, 3) ))

void __declspec(dllimport) log_body_misc(const char *module, const char *fmt, ...) LOG_CHECK_FMT;
void __declspec(dllimport) log_body_info(const char *module, const char *fmt, ...) LOG_CHECK_FMT;
void __declspec(dllimport) log_body_warning(const char *module, const char *fmt, ...) LOG_CHECK_FMT;
void __declspec(dllimport) log_body_fatal(const char *module, const char *fmt, ...) LOG_CHECK_FMT;

char *to_hex(const uint8_t *data, size_t data_len) {
  char *output = (char *) malloc(data_len * 3);

  char *current = output;
  for (size_t i = 0; i < data_len; i++) {
    current += sprintf(current, "%02x ", data[i]);
  }

  *(current - 1) = '\0';
  *current = '\0';

  return output;
}

uint8_t *find_pattern(uint8_t *data, size_t data_size, const uint8_t *pattern, const bool *pattern_mask, size_t pattern_size) {
  size_t i, j;
  bool pattern_found;

  for (i = 0; i < data_size - pattern_size; i++) {
    if (pattern_mask == NULL) {
      pattern_found = memcmp(&data[i], pattern, pattern_size) == 0;
    } else {
      pattern_found = true;

      for (j = 0; j < pattern_size; j++) {
        if (pattern_mask[j] && data[i + j] != pattern[j]) {
          pattern_found = false;
          break;
        }
      }
    }

    if (pattern_found) {
      log_info("pattern found at index %x size %d", i, pattern_size);

      return &data[i];
    }
  }

  return NULL;
}

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

const uint8_t packlist_omnilist_pattern[] = { 0x70, 0x61, 0x63, 0x6B, 0x6C, 0x69, 0x73, 0x74 };
const uint8_t packlist_omnilist_data[] = { 0x6F, 0x6D, 0x6E, 0x69 };

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
const uint8_t music_info_pattern[] = { 0x6D, 0x75, 0x73, 0x69, 0x63, 0x5F, 0x69, 0x6E, 0x66, 0x6F, 0x2E, 0x78, 0x6D, 0x6C };
const uint8_t music_omni_data[] = { 0x6F, 0x6D, 0x6E, 0x69 };

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

void do_patch(HANDLE process, const MODULEINFO *module_info, const struct patch_t *patch) {
  char *hex_data;
  uint8_t *addr, *target;
  DWORD old_protect;

  log_info("===== %s =====", patch->name);

  hex_data = to_hex(patch->pattern, patch->pattern_size);
  log_info("pattern: %s", hex_data);
  free(hex_data);

  if (patch->pattern_mask != NULL) {
    hex_data = to_hex((const uint8_t *) patch->pattern_mask, patch->pattern_size);
    log_info("mask   : %s", hex_data);
    free(hex_data);
  }

  addr = find_pattern(module_info->lpBaseOfDll, module_info->SizeOfImage, patch->pattern, patch->pattern_mask, patch->pattern_size);

  if (addr != NULL) {
    hex_data = to_hex(addr, patch->pattern_size);
    log_info("data: %s", hex_data);
    free(hex_data);

    target = &addr[patch->data_offset];

    if (!VirtualProtectEx(process, target, patch->data_size, PAGE_EXECUTE_READWRITE, &old_protect)) {
      log_fatal("VirtualProtectEx (rwx) failed: %08lx", GetLastError());
    }

    WriteProcessMemory(process, target, patch->data, patch->data_size, NULL);
    FlushInstructionCache(process, target, patch->data_size);

    if (!VirtualProtectEx(process, target, patch->data_size, old_protect, &old_protect)) {
      log_fatal("VirtualProtectEx (old) failed: %08lx", GetLastError());
    }

    log_info("%s applied at %p", patch->name, target);

    hex_data = to_hex(addr, patch->pattern_size);
    log_info("data: %s", hex_data);
    free(hex_data);
  } else {
    log_warning("could not find %s base address", patch->name);
  }
}

bool __declspec(dllexport) dll_entry_init(char *sid_code, void *app_config) {
  DWORD pid;
  HANDLE process;
  HMODULE jubeat_handle, music_db_handle;
  uint8_t *jubeat, *music_db;

  MODULEINFO jubeat_info, music_db_info;

  log_to_external(log_body_misc, log_body_info, log_body_warning, log_body_fatal);

  log_info("jubeat omnimix hook by Felix v" OMNIMIX_VERSION " (Build " __DATE__ " " __TIME__ ")");

  pid = GetCurrentProcessId();
  process = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE, FALSE, pid);

  jubeat_handle = GetModuleHandle("jubeat.dll");
  music_db_handle = GetModuleHandle("music_db.dll");

  jubeat = (uint8_t *) jubeat_handle;
  music_db = (uint8_t *) music_db_handle;

  log_info("jubeat.dll = %p, music_db.dll = %p", jubeat, music_db);
  log_info("sid_code = %s", sid_code);

  if (!GetModuleInformation(process, jubeat_handle, &jubeat_info, sizeof(jubeat_info)) ||
      !GetModuleInformation(process, music_db_handle, &music_db_info, sizeof(music_db_info)))
  {
    log_fatal("GetModuleInformation failed: %08lx", GetLastError());
  }

  log_info("jubeat image size: %ld", jubeat_info.SizeOfImage);

  do_patch(process, &jubeat_info, &tutorial_skip);
  do_patch(process, &jubeat_info, &select_timer_freeze);
  do_patch(process, &jubeat_info, &packlist_omnilist);
  do_patch(process, &music_db_info, &big_bnr_patch);
  do_patch(process, &music_db_info, &music_db_limit_1);
  do_patch(process, &music_db_info, &music_db_limit_2);
  do_patch(process, &music_db_info, &music_db_limit_3);
  do_patch(process, &music_db_info, &music_db_limit_4);
  do_patch(process, &music_db_info, &music_omni_patch);

  CloseHandle(process);

  sid_code[5] = 'X';

  SetEnvironmentVariableA("MB_MODEL", "----");

  return JB_DLL_ENTRY_INIT(sid_code, app_config);
  //return false;
}

bool __declspec(dllexport) dll_entry_main(void) {
  return JB_DLL_ENTRY_MAIN();
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
  return TRUE;
}
