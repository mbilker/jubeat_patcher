shared_library(
    'extend',
    name_prefix: '',
    include_directories: inc,
    implicit_include_directories: false,
    dependencies: [
        psapi_lib,
    ],
    link_with: [
        avs_1703_lib,
        gftools_lib,
        jubeat_lib,
        pattern_lib,
        pe_lib,
        util_lib,
    ],
    sources: [
        'extend.cc',
        'music_db.c',
        'music_db.h',
        'pkfs.c',
        'pkfs.h',
    ],
)
