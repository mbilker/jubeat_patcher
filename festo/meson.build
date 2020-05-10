shared_library(
    'omnimix',
    name_prefix: '',
    include_directories: inc,
    implicit_include_directories: false,
    dependencies: [
        psapi_lib,
    ],
    link_with: [
        avs_lib,
        jubeat_lib,
        pattern_lib,
        util_lib,
    ],
    sources: [
        'omnimix.c',
    ],
)
