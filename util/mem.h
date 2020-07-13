#pragma once

#include <windows.h>

#include <stdint.h>

#ifdef __cplusplus
#include <vector>

extern "C" {
#endif

void *memory_alloc_zeroed(size_t size);
void *memory_alloc(size_t size);
void *memory_realloc(void *mem, size_t size);
void memory_free(void *mem);

void memory_write(HANDLE process, void *target, const void *data, size_t data_size);
void memory_write_ptr(HANDLE process, void *target, const uintptr_t ptr);
void memory_set(HANDLE process, void *target, uint8_t data_value, size_t data_size);

#ifdef __cplusplus
};

inline void memory_write(HANDLE process, void *target, const std::vector<uint8_t> &data)
{
    memory_write(process, target, data.data(), data.size());
}
#endif
