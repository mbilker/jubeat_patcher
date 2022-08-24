festo_common_lib = static_library(
    'fest_common',
    include_directories: inc,
    dependencies: [
        minhook,
        capnhook.get_variable('hook_dep'),
    ],
    sources: [
        'bnr_hook.cc',
        'festo.cc',
    ],
)
