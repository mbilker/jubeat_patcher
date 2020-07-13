#define LOG_MODULE "mem"

#include <windows.h>

#include "util/log.h"

void *memory_alloc_zeroed(size_t size)
{
    void *mem;

    mem = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, size);

    if (mem == NULL) {
        log_fatal("memory_alloc_zeroed(%u) failed", size);

        return NULL;
    }

    return mem;
}

void *memory_alloc(size_t size)
{
    void *mem;

    mem = HeapAlloc(GetProcessHeap(), 0, size);

    if (mem == NULL) {
        log_fatal("memory_alloc(%u) failed", size);

        return NULL;
    }

    return mem;
}

void *memory_realloc(void *mem, size_t size)
{
    void *new_mem;

    new_mem = HeapReAlloc(GetProcessHeap(), 0, mem, size);

    if (new_mem == NULL) {
        log_fatal("memory_realloc(%p, %u) failed", mem, size);

        return NULL;
    }

    return new_mem;
}

void memory_free(void *mem)
{
    HeapFree(GetProcessHeap(), 0, mem);
}

static DWORD memory_make_rw(HANDLE process, void *target, size_t data_size)
{
    DWORD old_protect;

    if (!VirtualProtectEx(process, target, data_size, PAGE_EXECUTE_READWRITE, &old_protect)) {
        log_fatal("VirtualProtectEx (rwx) failed: 0x%08lx", GetLastError());
    }

    return old_protect;
}

static void memory_restore_old(HANDLE process, void *target, size_t data_size, DWORD old_protect)
{
    if (!VirtualProtectEx(process, target, data_size, old_protect, &old_protect)) {
        log_fatal("VirtualProtectEx (old) failed: 0x%08lx", GetLastError());
    }
}

void memory_write(HANDLE process, void *target, const void *data, size_t data_size)
{
    DWORD old_protect;

    old_protect = memory_make_rw(process, target, data_size);

    WriteProcessMemory(process, target, data, data_size, NULL);
    FlushInstructionCache(process, target, data_size);

    memory_restore_old(process, target, data_size, old_protect);
}

void memory_write_ptr(HANDLE process, void *target, const uintptr_t data)
{
    memory_write(process, target, &data, sizeof(data));
}

void memory_set(HANDLE process, void *target, uint8_t data_value, size_t data_size)
{
    void *buf;

    buf = memory_alloc(data_size);

    memset(buf, data_value, data_size);
    memory_write(process, target, buf, data_size);

    memory_free(buf);
}
