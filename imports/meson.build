subdir('avs2-core')

gftools_lib = custom_target(
    'gftools',
    output: 'libgftools.a',
    input: 'gftools.def',
    command: [
        'i686-w64-mingw32-dlltool',
        '-p',
        'jb',
        '-d',
        '@INPUT@',
        '-l',
        '@OUTPUT@',
    ],
)

jubeat_lib = custom_target(
    'jubeat',
    output: 'libjubeat.a',
    input: 'jubeat.def',
    command: [
        'i686-w64-mingw32-dlltool',
        '-p',
        'jb',
        '-d',
        '@INPUT@',
        '-l',
        '@OUTPUT@',
    ],
)
