#pragma once

// clang-format off
#include <windows.h>
#include <psapi.h>
// clang-format on

#include <span>

void bnr_hook_init(const MODULEINFO &jubeat_info, std::span<const char *> extra_paths);
