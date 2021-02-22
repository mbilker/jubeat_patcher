#define LOG_MODULE "pe"

// clang-format off
#include <windows.h>
// clang-format on

#include <stdbool.h>
#include <stdint.h>

#include "hook/pe.h"

#include "util/log.h"
#include "util/mem.h"

const pe_iid_t *
module_get_iid_for_name(HMODULE module, const char *target_module_name)
{
    const pe_iid_t *iid;
    const char *iid_name;

    iid = pe_iid_get_first(module);

    if (iid == NULL) {
        log_warning("failed to get import descriptor for module: 0x%08lx", GetLastError());
        return NULL;
    }

    while (iid != NULL) {
        iid_name = pe_iid_get_name(module, iid);

        if (_stricmp(iid_name, target_module_name) == 0) {
            break;
        }

        iid = pe_iid_get_next(module, iid);
    }

    if (iid == NULL) {
        log_warning("failed to find import descriptor for '%s'", target_module_name);
        return NULL;
    }

    log_misc("found import descriptor for '%s' at %p", target_module_name, iid);

    return iid;
}

void *iid_get_addr_for_name(
    HMODULE module,
    const pe_iid_t *iid,
    uint16_t ordinal,
    const char *name)
{
    struct pe_iat_entry entry;
    size_t i;

    memset(&entry, 0, sizeof(entry));

    i = 0;

    while (pe_iid_get_iat_entry(module, iid, i++, &entry) == S_OK) {
        if (ordinal != 0 && ordinal == entry.ordinal) {
            return entry.ppointer;
        } else if (name != NULL && entry.name != NULL && _stricmp(entry.name, name) == 0) {
            return entry.ppointer;
        }
    }

    return NULL;
}

void hook_iat(
    HANDLE process,
    HMODULE module,
    const char *target_module_name,
    const char *import_name,
    void *target_func_ptr)
{
    const pe_iid_t *iid;
    void *target;

    iid = module_get_iid_for_name(module, target_module_name);

    if (!iid) {
        return;
    }

    target = iid_get_addr_for_name(module, iid, 0, import_name);

    if (!target) {
        return;
    }

    memory_write(process, target, &target_func_ptr, sizeof(target));

    log_misc(
        "patched '%s'(%p) in '%s' with %p",
        import_name,
        target,
        target_module_name,
        target_func_ptr);
}

void hook_iat_ordinal(
    HANDLE process,
    HMODULE module,
    const char *target_module_name,
    uint16_t target_func_ordinal,
    void *target_func_ptr)
{
    const pe_iid_t *iid;
    void *target;

    iid = module_get_iid_for_name(module, target_module_name);

    if (!iid) {
        return;
    }

    target = iid_get_addr_for_name(module, iid, target_func_ordinal, NULL);

    if (!target) {
        return;
    }

    memory_write(process, target, &target_func_ptr, sizeof(target));

    log_misc(
        "patched '%d'(%p) in '%s' with %p",
        target_func_ordinal,
        target,
        target_module_name,
        target_func_ptr);
}
