#define LOG_MODULE "pe"

#include <windows.h>
#include <dbghelp.h>

#include <stdbool.h>
#include <stdint.h>

#include "util/log.h"
#include "util/mem.h"

const IMAGE_IMPORT_DESCRIPTOR *module_get_iid_for_name(
        HANDLE process,
        HMODULE module,
        const char *target_module_name)
{
    IMAGE_IMPORT_DESCRIPTOR *import_descriptor;
    unsigned long size;
    const char *module_name;

    import_descriptor = ImageDirectoryEntryToData(
            module,
            true,
            IMAGE_DIRECTORY_ENTRY_IMPORT,
            &size);

    if (import_descriptor == NULL) {
        log_warning("failed to get import descriptor for module: 0x%08lx", GetLastError());
        return NULL;
    }
    if (size == 0) {
        log_warning("no imports for module %p", module);
        return NULL;
    }

    while (import_descriptor->Name != 0) {
        module_name = (const char *) (((uintptr_t) module) + ((uintptr_t) import_descriptor->Name));

        if (_stricmp(module_name, target_module_name) == 0) {
            break;
        }

        import_descriptor++;
    }

    if (import_descriptor->Name == 0) {
        log_warning("failed to find import descriptor for '%s'", target_module_name);
        return NULL;
    }

    log_misc("found import descriptor for '%s' at %p", target_module_name, import_descriptor);

    return import_descriptor;
}

void *iid_get_addr_for_name(
        HMODULE module,
        const IMAGE_IMPORT_DESCRIPTOR *import_descriptor,
        uint16_t ordinal,
        const char *name)
{
    uintptr_t module_base;
    intptr_t *import_rvas;
    const IMAGE_IMPORT_BY_NAME *import;
    IMAGE_THUNK_DATA *thunk_data;
    size_t i;

    module_base = (uintptr_t) module;
    import_rvas = (intptr_t *) (module_base + ((uintptr_t) import_descriptor->OriginalFirstThunk));

    i = 0;

    while (true) {
        // Check if the end of the import list was reached
        if (import_rvas[i] == 0) {
            return NULL;
        }

        // Check if this import entry is an ordinal import
        if (import_rvas[i] & INTPTR_MIN) {
            if (ordinal != 0 && ordinal == (uint16_t) import_rvas[i]) {
                break;
            }
        } else {
            import = (const IMAGE_IMPORT_BY_NAME *) (module_base + ((uintptr_t) import_rvas[i]));

            // Check if this import entry matches the desired import name
            if (name != NULL && _stricmp(import->Name, name) == 0) {
                break;
            }
        }

        i++;
    }

    thunk_data = (IMAGE_THUNK_DATA *) (module_base + ((uintptr_t) import_descriptor->FirstThunk));

    return &thunk_data[i].u1.Function;
}

void hook_iat(
        HANDLE process,
        HMODULE module,
        const char *target_module_name,
        const char *import_name,
        void *target_func_ptr)
{
    const IMAGE_IMPORT_DESCRIPTOR *import_descriptor;
    void *target;

    import_descriptor = module_get_iid_for_name(process, module, target_module_name);

    if (!import_descriptor) {
        return;
    }

    target = iid_get_addr_for_name(module, import_descriptor, 0, import_name);

    if (!target) {
        return;
    }

    memory_write(process, target, &target_func_ptr, sizeof(target));

    log_misc("patched '%s'(%p) in '%s' with %p", import_name, target, target_module_name, target_func_ptr);
}
