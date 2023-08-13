pe_lib = static_library(
    'pe',
    include_directories: inc,
    implicit_include_directories: false,
    dependencies: [
        capnhook.get_variable('hook_dep'),
        minhook,
    ],
    sources: [
        'apphook.c',
        'iat.c',
    ],
)
