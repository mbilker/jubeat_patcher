project(
    'autopatch',
    'c',
    'cpp',
    version: '2.0',
    default_options: [
        'buildtype=release',
        'b_lto=true',
        'b_ndebug=if-release',
        'b_pch=false',
        'b_vscrt=mt',
        'c_std=c11',
        'c_winlibs=[]',
        'cpp_eh=s',
        'cpp_std=c++17',
    ],
)

cc = meson.get_compiler('c')

add_project_arguments(
    '-DWIN32_LEAN_AND_MEAN',
    '-DOMNIMIX_VERSION="2.0"',
    #'-DVERBOSE',
    language: [
        'c',
        'cpp',
    ]
)

if cc.get_argument_syntax() != 'msvc'
    add_project_arguments(
        '-fno-ident',
        '-ffunction-sections',
        '-fdata-sections',
        '-fvisibility=hidden',
        '-masm=intel',
        language: [
            'c',
            'cpp',
        ]
    )

    add_project_link_arguments(
        '-Wl,--exclude-all-symbols',
        '-Wl,--gc-sections',
        '-Wl,--no-undefined',
        '-static',
        '-static-libgcc',
        language: [
            'c',
            'cpp',
        ]
    )
endif

dbghelp_lib = cc.find_library('dbghelp')
psapi_lib = cc.find_library('psapi')

inc = include_directories('.')

subdir('imports')

subdir('util')
subdir('pattern')
subdir('pe')

#subdir('qubell')
subdir('clan')
subdir('festo')
