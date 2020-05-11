avs_1508_lib = custom_target(
    'avs_1508',
    output: 'libavs_1508.a',
    input: 'avs_1508.def',
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

avs_1703_lib = custom_target(
    'avs_1703',
    output: 'libavs_1703.a',
    input: 'avs_1703.def',
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
