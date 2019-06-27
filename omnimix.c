#include <windows.h>
#include <iphlpapi.h>
#include <stdbool.h>
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

bool __declspec(dllexport) dll_entry_init(char *sid_code, void *app_config) {
  DWORD pid;
  HANDLE process;
  HMODULE jubeat, music_db;

  log_to_external(log_body_misc, log_body_info, log_body_warning, log_body_fatal);

  log_info("jubeat omnimix hook by Felix v" OMNIMIX_VERSION " (Build " __DATE__ " " __TIME__ ")");

  pid = GetCurrentProcessId();
  process = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE, FALSE, pid);
  jubeat = GetModuleHandle("jubeat.dll");
  music_db = GetModuleHandle("music_db.dll");

  log_info("jubeat.dll = %p, music_db.dll = %p", jubeat, music_db);
  log_info("sid_code = %s", sid_code);

  CloseHandle(process);

  sid_code[5] = 'X';

  SetEnvironmentVariableA("MB_MODEL", "----");

  return JB_DLL_ENTRY_INIT(sid_code, app_config);
}

bool __declspec(dllexport) dll_entry_main(void) {
  return JB_DLL_ENTRY_MAIN();
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
  return TRUE;
}
