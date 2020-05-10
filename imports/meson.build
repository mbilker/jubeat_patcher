avs_lib = custom_target(
    'avs',
    output: 'libavs.a',
    input: 'avs.def',
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
