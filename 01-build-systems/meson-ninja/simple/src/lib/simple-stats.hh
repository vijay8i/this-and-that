#ifndef SIMPLE_STATS_H
#define SIMPLE_STATS_H

// include *only* the header files that provide the types used by 
// the publicly exposed functions and interfaces
#include <vector>

#define STATS_API  
#ifdef _WIN32
  #if defined(STATS_SHARED_LIBRARY)
    #if defined(STATS_EXPORTS)
      // #undef STATS_API
      #define STATS_API __declspec(dllexport)
    #else
      // #undef STATS_API
      #define STATS_API __declspec(dllimport)
    #endif
  #endif
#endif

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
