#pragma once

#include <windows.h>

#ifdef __cplusplus

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"
#endif

template<typename T>
inline T get_proc_address(HMODULE module, const char *proc_name)
{
    return reinterpret_cast<T>(GetProcAddress(module, proc_name));
}

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

#endif
