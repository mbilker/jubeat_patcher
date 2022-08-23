#define LOG_MODULE "patch"

#include "patch.h"

#include "pattern/pattern.h"

#include "util/log.h"
#include "util/mem.h"

void do_patch(HANDLE process, const MODULEINFO &module_info, const struct patch_t &patch)
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
