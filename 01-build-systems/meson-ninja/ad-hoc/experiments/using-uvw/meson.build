# run as: CC=`which clang` CC_FOR_BUILD=`which clang` CXX=`which clang++` CXX_FOR_BUILD=`which clang++` meson setup --reconfigure build
# or: meson setup --native-file ./native.ini --reconfigure build
project(
  'using_uvw', 
  'cpp', 
  version: '3.5.0', 
  default_options: ['cpp_std=c++23']
)

fs = import('fs')

# set the compiler to use (doesn't work!)
# executable doesn't accept environment :-(
# env = environment()
# env.set('CC', '/opt/local/bin/clang++')
# env.set('CC_FOR_BUILD', '/opt/local/bin/clang++')
# env.set('CXX', '/opt/local/bin/clang++')
# env.set('CXX_FOR_BUILD', '/opt/local/bin/clang++')



# spec out 'uvw' and 'uv dependencies
local_lib_dir = join_paths(
  meson.current_source_dir(), 
  '../../3p/local/lib'
)

# Manually specify the uv dependency
libuv_includes = include_directories(
  fs.relative_to('../../3p/local/include', meson.current_source_dir())
)
libuv_dep = declare_dependency(
  include_directories: libuv_includes,
  link_args: ['-L' + local_lib_dir, '-luv']
)

# Manually specify the uvw dependency, it shares the 'uv' library
libuvw_includes = include_directories(
  fs.relative_to('../../3p/uvw/src/', meson.current_source_dir())
)
uvw_dep = declare_dependency(
  include_directories: libuvw_includes,
  link_args: ['-L' + local_lib_dir, '-luvw']
)

# using install_rpath doesn't work, so ignore the warning you get
# from meson
executable(
  'main', 'main.cpp', 
  dependencies: [uvw_dep, libuv_dep],
  # install_rpath: '/opt/local/libexec/llvm-18/lib',
  link_args: ['-L/opt/local/libexec/llvm-18/lib/', '-Wl,-rpath,/opt/local/libexec/llvm-18/lib']
)
