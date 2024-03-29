#define LOG_MODULE "bnr-hook"

#include <vector>

#include "pattern/pattern.h"

#include "util/log.h"

#include "MinHook.h"

#include "bnr_hook.h"
#include "festo.h"

// dunno how reliable this will be - it's the call to crc32 just near fn start
static const uint8_t d3_load_pattern[] = {
    0xC7, 0x45, 0xF4, 0xFF, 0xFF, 0xFF, 0xFF, 0xE8, 0x77, 0x88, 0x00, 0x00, 0x8B,
};
static const ssize_t d3_load_offset = -0x1D;

static std::vector<const char *> EXTRA_PATHS;

static int (__fastcall *d3_package_load)(const char *name);

static int __fastcall hook_d3_package_load(const char *name)
{
    // log_info("d3_package_load(\"%s\")", name);

    // loading banners, add our own
    if (strcmp(name, "L44_BNR_BIG_ID99999999") == 0) {
        // log_info("Loading extra omni banners");
        for (const char *banner : EXTRA_PATHS) {
            log_info("... %s", banner);
            d3_package_load(banner);
        }
    }
    // play marker textures
    if (strcmp(name, "L44_TM_BANNER") == 0) {
        // log_info("... %s", "Loading extra marker textures");
        log_info("... %s", "L44_TM_BANNER_OM");
        d3_package_load("L44_TM_BANNER_OM");
    }
    // play background textures (let's load explcit unique rather than bundled)
    if (strcmp(name, "L44FO_PLAY_BACKGROUND") == 0) {
        // log_info("... %s", "Loading extra bg textures");
        log_info("... %s", "L44FO_PLAY_BACKGROUND_OM");
        d3_package_load("L44FO_PLAY_BACKGROUND_OM");
    }
    if (strcmp(name, "L44FO_STG_BG_CHANGE") == 0) {
        // log_info("... %s", "Loading extra bg textures");
        log_info("... %s", "L44FO_STG_BG_CHANGE_OM");
        d3_package_load("L44FO_STG_BG_CHANGE_OM");
    }

    return d3_package_load(name);
}

void bnr_hook_init(const MODULEINFO &jubeat_info, std::vector<const char *> extra_paths)
{
    EXTRA_PATHS = std::move(extra_paths);

    void *d3_package_load_loc = find_pattern(
        reinterpret_cast<uint8_t *>(jubeat_info.lpBaseOfDll),
        jubeat_info.SizeOfImage,
        d3_load_pattern,
        nullptr,
        std::size(d3_load_pattern)) +
        d3_load_offset;

    MH_Initialize();
    MH_CreateHook(
        d3_package_load_loc,
        reinterpret_cast<void *>(hook_d3_package_load),
        reinterpret_cast<void **>(&d3_package_load));
    MH_EnableHook(MH_ALL_HOOKS);
}
