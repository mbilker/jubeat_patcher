shared_library(
    'ultimate',
    name_prefix: '',
    include_directories: inc,
    implicit_include_directories: false,
    vs_module_defs: 'ultimate.def',
    dependencies: [
        capnhook.get_variable('hook_dep'),
        psapi_lib,
    ],
    link_with: [
        avs_1703_lib,
        gftools_lib,
        jubeat_lib,
        pattern_lib,
        pe_lib,
        util_lib,
        festo_common_lib,
    ],
    sources: [
        'ultimate.cc',
        'music_db.cc',
        'pkfs.c',
    ],
)
