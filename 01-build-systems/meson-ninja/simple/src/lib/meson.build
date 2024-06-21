ss_sources = ['simple-stats.cc']

ss_shared = shared_library(
  'ss-shared', 
  ss_sources, 
  cpp_args: '-DSTATS_API_BUILD_AS_SHARED_LIB'
)

ss_static = static_library(
  'ss-static', 
  ss_sources, 
  cpp_args: '-DSTATS_API_BUILD_AS_STATIC_LIB'
)
