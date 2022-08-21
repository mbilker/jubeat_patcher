#define LOG_MODULE "pe"

// clang-format off
#include <windows.h>
// clang-format on

#include <stdbool.h>
#include <stdint.h>

#include "hook/pe.h"
#include "hook/table.h"

#include "util/log.h"
#include "util/mem.h"

const pe_iid_t *module_get_iid_for_name(HMODULE module, const char *target_module_name)
{
    const pe_iid_t *iid;
    const char *iid_name;

    iid = pe_iid_get_first(module);

    if (iid == NULL) {
        log_warning("no imports for module %p", module);
        return NULL;
    }

    for (; iid != NULL; iid = pe_iid_get_next(module, iid)) {
        iid_name = pe_iid_get_name(module, iid);

        if (_stricmp(iid_name, target_module_name) == 0) {
            break;
        }
    }

    if (iid == NULL) {
        log_warning("failed to find import descriptor for '%s'", target_module_name);
        return NULL;
    }

    log_misc("found import descriptor for '%s' at %p", target_module_name, iid);

    return iid;
}

void *iid_get_addr_for_name(HMODULE module, const pe_iid_t *iid, uint16_t ordinal, const char *name)
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

static bool hook_table_match_proc(const struct pe_iat_entry *entry, const struct hook_symbol *sym)
{
    if (sym->name != NULL && entry->name != NULL && _stricmp(sym->name, entry->name) == 0) {
        return true;
    }

    if (sym->ordinal != 0 && sym->ordinal == entry->ordinal) {
        return true;
    }

    return false;
}

// Based on capnhook's `hook_table_apply` with logging added
void iat_hook_table_apply(
    HANDLE process,
    HMODULE module,
    const char *target_module_name,
    const struct hook_symbol *syms,
    size_t num_syms)
{
    const pe_iid_t *iid;
    struct pe_iat_entry entry;
    size_t i;
    size_t j;
    const struct hook_symbol *sym;

    log_assert(module != NULL);
    log_assert(target_module_name != NULL);
    log_assert(syms != NULL || num_syms == 0);

    iid = module_get_iid_for_name(module, target_module_name);

    if (!iid) {
        return;
    }

    i = 0;

    while (pe_iid_get_iat_entry(module, iid, i++, &entry) == S_OK) {
        for (j = 0; j < num_syms; j++) {
            sym = &syms[j];

            if (hook_table_match_proc(&entry, sym)) {
                if (sym->link != NULL && *sym->link == NULL) {
                    *sym->link = *entry.ppointer;
                }

                memory_write(process, entry.ppointer, &sym->patch, sizeof(sym->patch));

                log_misc(
                    "patched '%s'(%p) in '%s' with %p",
                    sym->name,
                    entry.ppointer,
                    target_module_name,
                    sym->patch);
            }
        }
    }
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
