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
        'cpp_eh=none',
        'cpp_std=c++20',
        'warning_level=2',
    ],
)

cc = meson.get_compiler('c')

add_project_arguments(
    '-DWIN32_LEAN_AND_MEAN',
    '-DOMNIMIX_VERSION="4.0.0_RC4"',
    #'-DVERBOSE',
    language: [
        'c',
        'cpp',
    ],
)

if cc.get_argument_syntax() != 'msvc'
    add_project_arguments(
        '-fno-ident',
        '-ffunction-sections',
        '-fdata-sections',
        '-fvisibility=hidden',
        language: [
            'c',
            'cpp',
        ],
    )
    add_project_arguments(
        '-Wold-style-cast',
        language: [
            'cpp',
        ],
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
        ],
    )
endif

psapi_lib = cc.find_library('psapi')

inc = include_directories('.')
capnhook = subproject('capnhook', default_options: ['warning_level=0'])

cmake = import('cmake')
minhook_opts = cmake.subproject_options()
minhook_opts.set_override_option('warning_level', '0')
minhook_sub = cmake.subproject('minhook', options: minhook_opts)
minhook = minhook_sub.dependency('minhook')

subdir('imports')

subdir('util')
subdir('pattern')
subdir('pe')

subdir('clan')
subdir('festo')
