#ifndef SIMPLE_STATS_H
#define SIMPLE_STATS_H

// include *only* the header files that provide the types used by 
// the publicly exposed functions and interfaces
#include <vector>

#if defined _WIN32 || defined __CYGWIN__
  #define STATS_API_Export __declspec(dllexport)
  #define STATS_API_Import __declspec(dllimport)
#elif __GNUC__ >= 4
  #define STATS_API_Export __attribute__ ((visibility ("default")))
  #define STATS_API_Import __attribute__ ((visibility ("default")))
#else
  #define STATS_API_Export
  #define STATS_API_Import
#endif

#if defined (STATS_API_BUILD_AS_STATIC_LIB)
# if !defined (STATS_API_IS_DLL)
#   define STATS_API_IS_DLL 0
# endif /* ! STATS_API_IS_DLL */
#else
# if !defined (STATS_API_IS_DLL)
#   define STATS_API_IS_DLL 1
# endif /* ! STATS_API_IS_DLL */
#endif /* STATS_API_BUILD_AS_STATIC_LIB */

#if defined (STATS_API_IS_DLL)
#  if (STATS_API_IS_DLL == 1)
#    if defined (STATS_API_BUILD_AS_SHARED_LIB)
// #      pragma message ( "compiling .so")
#      define STATS_API STATS_API_Export
#    else
// #      pragma message ( "importing .so")
#      define STATS_API STATS_API_Import
#    endif /* STATS_API_BUILD_AS_SHARED_LIB */
#  else
#    define STATS_API
#  endif   /* ! STATS_API_IS_DLL == 1 */
#else
#  define STATS_API
#endif     /* STATS_API_IS_DLL */

// define types without exposing the entire namespace of std or
// any other dependent library
namespace ss {
  /// vector of integers
  using voi = std::vector<int>;

  STATS_API auto sum(const voi& nums) -> int;
  STATS_API auto average(const voi& nums) -> double;
  STATS_API auto median(voi& nums) -> double;
}

#endif /* SIMPLE_STATS_H */
