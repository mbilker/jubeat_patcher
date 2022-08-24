#pragma once

// clang-format off
#include <windows.h>
#include <psapi.h>
// clang-format on

#include <vector>

// TODO: replace `std::vector` with `std::span` once GitHub Actions supports an Ubuntu version
// that has a newer MinGW revision, or switch to `msys2/setup-msys2` and build under the
// Windows MSYS2 environment with the MinGW toolchain provided by the MSYS2 repositories
void bnr_hook_init(const MODULEINFO &jubeat_info, std::vector<const char *> extra_paths);
