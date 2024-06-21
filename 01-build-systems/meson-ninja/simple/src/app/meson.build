ss_include = include_directories('../lib')

static_dep = declare_dependency(
  include_directories: ss_include,
  compile_args: '-DSTATS_API_IS_DLL=0',
  link_with: ss_static
)

# using install_rpath doesn't work, so ignore the warning from meson
# or educate me on how to make it work!
executable(
  'app-static',
  'main.cc',
  dependencies: static_dep,
  link_args: ['-L/opt/local/libexec/llvm-18/lib/', '-Wl,-rpath,/opt/local/libexec/llvm-18/lib']
)

shared_dep = declare_dependency(
  include_directories: ss_include,
  compile_args: '-DSTATS_API_IS_DLL=1',
  link_with: ss_shared
)

# using install_rpath doesn't work, so ignore the warning from meson
# or educate me on how to make it work!
executable(
  'app-shared',
  'main.cc',
  dependencies: shared_dep,
  link_args: ['-L/opt/local/libexec/llvm-18/lib/', '-Wl,-rpath,/opt/local/libexec/llvm-18/lib']
)
