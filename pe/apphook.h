// Taken from bemanitools and modified to work here

#pragma once

#include <stdbool.h>
#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef bool (*dll_entry_init_t)(char *, void *);
typedef bool (*dll_entry_main_t)(void);

void app_hook_init(dll_entry_init_t init, dll_entry_main_t main_);
bool app_hook_invoke_init(char *sidcode, void *config);
bool app_hook_invoke_main(void);
HMODULE app_hook_get_dll_handle(void);

#ifdef __cplusplus
};
#endif
