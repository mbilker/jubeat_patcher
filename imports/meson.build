subdir('avs2-core')

if cc.get_argument_syntax() == 'msvc'
    config_lib = custom_target(
        'config',
        output: 'config.lib',
        input: 'config.def',
        command: [
            'lib.exe',
            '/DEF:@INPUT@',
            '/MACHINE:x86',
            '/OUT:@OUTPUT@',
        ],
    )

    gftools_lib = custom_target(
        'gftools',
        output: 'gftools.lib',
        input: 'gftools.def',
        command: [
            'lib.exe',
            '/DEF:@INPUT@',
            '/MACHINE:x86',
            '/OUT:@OUTPUT@',
        ],
    )

    jubeat_lib = custom_target(
        'jubeat',
        output: 'jubeat.lib',
        input: 'jubeat.def',
        command: [
            'lib.exe',
            '/DEF:@INPUT@',
            '/MACHINE:x86',
            '/OUT:@OUTPUT@',
        ],
    )
else
    config_lib = custom_target(
        'config',
        output: 'libconfig.a',
        input: 'config.def',
        command: [
            'i686-w64-mingw32-dlltool',
            '-d',
            '@INPUT@',
            '-l',
            '@OUTPUT@',
        ],
    )

    gftools_lib = custom_target(
        'gftools',
        output: 'libgftools.a',
        input: 'gftools.def',
        command: [
            'i686-w64-mingw32-dlltool',
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
            '-d',
            '@INPUT@',
            '-l',
            '@OUTPUT@',
        ],
    )
endif
