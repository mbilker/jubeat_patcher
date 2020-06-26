#pragma once

#include <windows.h>

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void do_relative_jmp(HANDLE process, void *target, const void *new_addr);
void do_absolute_jmp(HANDLE process, void *target, const uint32_t new_addr);

#ifdef __cplusplus
};
#endif
