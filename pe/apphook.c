// Taken from bemanitools and modified to work with both btools *and* spice

#define LOG_MODULE "pe"

// clang-format off
#include <windows.h>
#include <psapi.h>
// clang-format on

#include "apphook.h"

#include "util/defs.h"
#include "util/log.h"
#include "util/str.h"

#include "MinHook.h"

static dll_entry_init_t next_dll_entry_init;
static dll_entry_main_t next_dll_entry_main;
static HMODULE dll_handle;

void app_hook_init(dll_entry_init_t init, dll_entry_main_t main_)
{
    // Bemanitools works in this order:
    //   load game DLL
    //   load hook DLLs
    //   GetProcAddress(game_dll, "dll_entry_init");
    // Unfortunately, spice works in this order:
    //   load game DLL
    //   GetProcAddress(game_dll, "dll_entry_init");
    //   load hook DLLs

    // Because of this, we can't hook GetProcAddress. We *also* can't use IAT
    // hooks, because spice already took an address. Instead, let's just iterate
    // every loaded module until we find one with dll_entry_init and
    // dll_entry_main

    HANDLE process;
    DWORD pid, modules_sz;
    HMODULE *modules;

    pid = GetCurrentProcessId();
    process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, false, pid);
    if (!process) {
        log_fatal("OpenProcess failed: 0x%08lx", GetLastError());
    }

    EnumProcessModules(process, NULL, 0, &modules_sz);
    modules = malloc(modules_sz);
    if (!EnumProcessModules(process, modules, modules_sz, &modules_sz)) {
        log_fatal("EnumProcessModules failed: 0x%08lx", GetLastError());
    }

    for (size_t i = 0; i < (modules_sz / sizeof(HMODULE)); i++) {
        if (!GetProcAddress(modules[i], "dll_entry_init")) {
            continue;
        }
        dll_handle = modules[i];

        // this is lazy, but works. log_info can't take wchars, minhook needs them
        wchar_t mod_name_w[MAX_PATH];
        char mod_name_a[MAX_PATH];

        if (!GetModuleBaseNameW(process, dll_handle, mod_name_w, MAX_PATH) ||
            !GetModuleBaseNameA(process, dll_handle, mod_name_a, MAX_PATH))
        {
            log_fatal("GetModuleBaseName failed: 0x%08lx", GetLastError());
        }

        if (init)
            MH_CreateHookApi(mod_name_w, "dll_entry_init", init, (void **) &next_dll_entry_init);
        if (main_)
            MH_CreateHookApi(mod_name_w, "dll_entry_main", init, (void **) &next_dll_entry_main);

        log_info("Found game DLL \"%s\" at %p", mod_name_a, dll_handle);
    }

    if (!dll_handle) {
        log_fatal("Could not find main game DLL in memory (nothing exported dll_entry_init)");
    }

    CloseHandle(process);
    free(modules);
}

bool app_hook_invoke_init(char *sidcode, void *config)
{
    log_assert(sidcode != NULL);
    log_assert(config != NULL);

    return next_dll_entry_init(sidcode, config);
}

bool app_hook_invoke_main(void)
{
    return next_dll_entry_main();
}

HMODULE app_hook_get_dll_handle(void)
{
    return dll_handle;
}
