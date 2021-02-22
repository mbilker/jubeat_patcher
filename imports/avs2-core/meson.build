if cc.get_argument_syntax() == 'msvc'
    avs_1508_lib = custom_target(
        'avs_1508',
        output: 'avs_1508.lib',
        input: 'avs_1508.def',
        command: [
            'lib.exe',
            '/DEF:@INPUT@',
            '/MACHINE:x86',
            '/OUT:@OUTPUT@',
        ],
    )

    avs_1703_lib = custom_target(
        'avs_1703',
        output: 'avs_1703.a',
        input: 'avs_1703.def',
        command: [
            'lib.exe',
            '/DEF:@INPUT@',
            '/MACHINE:x86',
            '/OUT:@OUTPUT@',
        ],
    )
else
    avs_1508_lib = custom_target(
        'avs_1508',
        output: 'libavs_1508.a',
        input: 'avs_1508.def',
        command: [
            'i686-w64-mingw32-dlltool',
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
            '-d',
            '@INPUT@',
            '-l',
            '@OUTPUT@',
        ],
    )
endif
