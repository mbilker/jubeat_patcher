#define LOG_MODULE "bnr-hook"

#include <cstdio>
#include <vector>
#include <unordered_map>

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

static std::unordered_map<const char*, std::vector<const char*>> extra_banners;

static int (__fastcall *d3_package_load)(const char *name);

static int __fastcall hook_d3_package_load(const char *name)
{
    // log_info("d3_package_load(\"%s\")", name);

    // Sample the D3 pool caps before loading. If this package is the one
    // that pushes us over, the "pre" snapshot tells us the name & prior
    // state; the "post" snapshot tells us the delta this package cost.
    {
        char ctx[128];
        snprintf(ctx, sizeof(ctx), "pre  d3_package_load(\"%s\")", name);
        festo_d3_guard_check(ctx);
    }

    // todo: custom hashing function for marginally faster lookup?
    for(auto &entry : extra_banners) {
        if(strcmp(name, entry.first) == 0) {
            log_info("loading %s", name);

            for (auto banner : entry.second) {
                log_info("... %s", banner);
                d3_package_load(banner);
            }
        }
    }

    int ret = d3_package_load(name);

    {
        char ctx[128];
        snprintf(ctx, sizeof(ctx), "post d3_package_load(\"%s\")", name);
        festo_d3_guard_check(ctx);
    }

    return ret;
}

void bnr_hook_init(const MODULEINFO &jubeat_info)
{
    void *d3_package_load_loc = find_pattern(
        reinterpret_cast<uint8_t *>(jubeat_info.lpBaseOfDll),
        jubeat_info.SizeOfImage,
        d3_load_pattern,
        nullptr,
        std::size(d3_load_pattern)) +
        d3_load_offset;

    MH_CreateHook(
        d3_package_load_loc,
        reinterpret_cast<void *>(hook_d3_package_load),
        reinterpret_cast<void **>(&d3_package_load));
}

void bnr_hook_add_paths(const char * trigger_file, std::vector<const char*> extra_paths)
{
    extra_banners[trigger_file] = extra_paths;
}
