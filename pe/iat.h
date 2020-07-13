#pragma once

// clang-format off
#include <windows.h>
// clang-format on

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

const IMAGE_IMPORT_DESCRIPTOR *
module_get_iid_for_name(HANDLE process, HMODULE module, const char *target_module_name);
void *iid_get_addr_for_name(
    HMODULE module,
    const IMAGE_IMPORT_DESCRIPTOR *import_descriptor,
    uint16_t ordinal,
    const char *name);
void hook_iat(
    HANDLE process,
    HMODULE module,
    const char *target_module_name,
    const char *import_name,
    void *target_func_ptr);
void hook_iat_ordinal(
    HANDLE process,
    HMODULE module,
    const char *target_module_name,
    uint16_t target_func_ordinal,
    void *target_func_ptr);

#ifdef __cplusplus
};
#endif
