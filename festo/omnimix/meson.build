shared_library(
    'omnimix',
    name_prefix: '',
    include_directories: inc,
    implicit_include_directories: false,
    dependencies: [
        psapi_lib,
        minhook,
    ],
    link_with: [
        avs_1703_lib,
        festo_common_lib,
        pe_lib,
        util_lib,
    ],
    sources: [
        'omnimix.cc',
    ],
)
