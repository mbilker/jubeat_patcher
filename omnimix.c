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

void tutorial_skip(uint8_t *target, HANDLE process) {
  const uint8_t data[5] = { 0xE9, 0x90, 0x00, 0x00, 0x00 };
  //WriteProcessMemory(process, &jubeat[0xD0A67], &data, sizeof(data), NULL);
  WriteProcessMemory(process, target, &data, sizeof(data), NULL);
  FlushInstructionCache(process, target, sizeof(data));
}

void select_timer_freeze(uint8_t *jubeat, HANDLE process) {
  const uint8_t data[5] = { 0xEB };
  WriteProcessMemory(process, &jubeat[0xA6499], &data, sizeof(data), NULL);
}

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

uint8_t *find_pattern(uint8_t *data, size_t data_size, const uint8_t *pattern, size_t pattern_size) {
  size_t i, j;

  char *hex_data = to_hex(pattern, pattern_size);
  log_info("pattern: %s", hex_data);
  free(hex_data);

  for (i = 0; i < data_size - pattern_size; i++) {
    for (j = 0; j < pattern_size; j++) {
      if (data[i + j] != pattern[j]) {
        break;
      }
    }

    if (j == pattern_size) {
      log_info("pattern found at index %08x size %d", i, pattern_size);

      return &data[i];
    }
  }

  return NULL;
}

bool __declspec(dllexport) dll_entry_init(char *sid_code, void *app_config) {
  DWORD pid;
  HANDLE process;
  HMODULE jubeat_handle, music_db_handle;
  uint8_t *jubeat, *music_db;

  MODULEINFO jubeat_info;

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

  if (!GetModuleInformation(process, jubeat_handle, &jubeat_info, sizeof(jubeat_info))) {
    log_fatal("GetModuleInformation failed: %08lx", GetLastError());
  }

  log_info("jubeat image size: %ld", jubeat_info.SizeOfImage);

  const uint8_t pattern[] = { 0x3D, 0x21, 0x00, 0x00, 0x80, 0x75, 0x4A, 0x57, 0x68, 0x00, 0x00, 0x60 };
  uint8_t *addr = find_pattern(jubeat_info.lpBaseOfDll, jubeat_info.SizeOfImage, pattern, sizeof(pattern));

  if (addr != NULL) {
    char *hex_data = to_hex(addr, sizeof(pattern));
    log_info("data: %s", hex_data);
    free(hex_data);

    log_info("tutorial skip applied at %p (%02x)", &addr[5], addr[5]);
    tutorial_skip(&addr[5], process);

    hex_data = to_hex(addr, sizeof(pattern));
    log_info("data: %s", hex_data);
    free(hex_data);
  } else {
    log_warning("could not find tutorial skip base address");
  }

  select_timer_freeze(jubeat, process);

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
