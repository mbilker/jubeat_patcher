// clang-format off
#include <windows.h>
#include <psapi.h>
// clang-format on

#include <vector>

void bnr_hook_init(LPMODULEINFO jubeat_info, std::vector<const char*> _extra_paths);
