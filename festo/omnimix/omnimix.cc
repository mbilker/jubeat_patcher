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

#include "MinHook.h"

struct patch_t {
    const char *name;
    const std::vector<uint8_t> pattern;
    // MSVC does not allow the `(const bool[]) { ... }` initializer and `std::vector<bool>` is a
    // specialization, so use `std::vector<uint8_t>` instead
    const std::vector<uint8_t> pattern_mask;
    const std::vector<uint8_t> data;
    ssize_t data_offset;
};

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

const struct patch_t song_unlock_patch {
    .name = "song unlock patch",
    .pattern = { 0x74, 0x0A, 0x8B, 0x4C },
    .data = { 0x90, 0x90 },
    .data_offset = 0,
};

// clang-format on

// dunno how reliable this will be - it's the call to crc32 just near fn start
const uint8_t d3_load_pattern[] = { 0xC7, 0x45, 0xF4, 0xFF, 0xFF, 0xFF, 0xFF, 0xE8, 0x77, 0x88, 0x00, 0x00, 0x8B };
const ssize_t d3_load_offset = -0x1D;
const std::vector<const char*> extra_banners = {
    "L44FO_BNR_J_OM_001",
    "L44FO_BNR_J_OM_002",
};

int (__fastcall *d3_package_load)(const char *name);

int __fastcall hook_d3_package_load(const char *name)
{
    // log_info("d3_package_load(\"%s\")", name);

    // loading banners, add our own
    if(strcmp(name, "L44_BNR_BIG_ID99999999") == 0) {
        // log_info("Loading extra omni banners");
        for(auto banner : extra_banners) {
            log_info("... %s", banner);
            d3_package_load(banner);
        }
    }

    return d3_package_load(name);
}

static void do_patch(HANDLE process, const MODULEINFO &module_info, const struct patch_t &patch)
{
#ifdef VERBOSE
    char *hex_data;
#endif
    uint8_t *addr, *target;

#ifdef VERBOSE
    log_info("===== %s =====", patch.name);

    hex_data = to_hex(patch.pattern.data(), patch.pattern.size());
    log_info("pattern: %s", hex_data);
    free(hex_data);

    if (!patch.pattern_mask.empty()) {
        hex_data = to_hex(patch.pattern_mask.data(), patch.pattern.size());
        log_info("mask   : %s", hex_data);
        free(hex_data);
    }
#endif

    addr = find_pattern(
        reinterpret_cast<uint8_t *>(module_info.lpBaseOfDll),
        module_info.SizeOfImage,
        patch.pattern.data(),
        reinterpret_cast<const bool *>(patch.pattern_mask.data()),
        patch.pattern.size());

    if (addr != nullptr) {
#ifdef VERBOSE
        hex_data = to_hex(addr, patch.pattern.size());
        log_info("data: %s", hex_data);
        free(hex_data);
#endif

        target = &addr[patch.data_offset];

        memory_write(process, target, patch.data);

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

extern "C" DLL_EXPORT bool __cdecl omnimix_dll_entry_init(char *sid_code, void *app_config)
{
    DWORD pid;
    HANDLE process;
    HMODULE jubeat_handle, music_db_handle;
    MODULEINFO jubeat_info, music_db_info;
#ifdef VERBOSE
    uint8_t *jubeat, *music_db;
#endif

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

    do_patch(process, jubeat_info, packlist);
    do_patch(process, music_db_info, music_db_limit_1);
    do_patch(process, music_db_info, music_db_limit_2);
    do_patch(process, music_db_info, music_db_limit_3);
    do_patch(process, music_db_info, music_db_limit_4);
    do_patch(process, music_db_info, music_omni_patch);
    do_patch(process, music_db_info, song_unlock_patch);

    void *d3_package_load_loc = find_pattern(
        reinterpret_cast<uint8_t *>(jubeat_info.lpBaseOfDll),
        jubeat_info.SizeOfImage,
        d3_load_pattern,
        nullptr,
        std::size(d3_load_pattern)) + d3_load_offset;

    MH_Initialize();
    MH_CreateHook(d3_package_load_loc, (void*)hook_d3_package_load, (void**)&d3_package_load);
    MH_EnableHook(MH_ALL_HOOKS);

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
    return TRUE;
}
