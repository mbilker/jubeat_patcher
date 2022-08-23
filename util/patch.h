#ifndef UTIL_PATCH_H
#define UTIL_PATCH_H

// clang-format off
#include <windows.h>
#include <psapi.h>
// clang-format on

#include <stdint.h>
#include <stdlib.h>

#include <vector>

// use vectors, so we get sizes for free
struct patch_t {
    const char *name;
    const std::vector<uint8_t> pattern;
    // MSVC does not allow the `(const bool[]) { ... }` initializer and `std::vector<bool>` is a
    // specialization, so use `std::vector<uint8_t>` instead
    const std::vector<uint8_t> pattern_mask {};
    const std::vector<uint8_t> data;
    ssize_t data_offset;
};

void do_patch(HANDLE process, const MODULEINFO &module_info, const struct patch_t &patch);

#endif // UTIL_PATCH_H
