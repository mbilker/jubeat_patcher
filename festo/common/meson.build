festo_common_lib = static_library(
    'festo_common',
    include_directories: inc,
    dependencies: [
        capnhook.get_variable('hook_dep'),
        minhook,
    ],
    link_with: [
        pattern_lib,
        pe_lib,
        util_lib,
    ],
    sources: [
        'bnr_hook.cc',
        'festo.cc',
    ],
)
