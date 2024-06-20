#include "simple-stats.hh"

#include <algorithm>
#include <ranges>
#include <numeric>

#if defined(STATS_SHARED_LIBRARY)
#pragma message ( "compiling .lib")
#else
#pragma message ( "compiling .so")
#endif
namespace ss {
  using namespace std;

  auto sum(const voi& nums) -> int {
      return accumulate(nums.begin(), nums.end(), 0);
  }

  auto average(const voi& nums) -> double {
      return static_cast<double>(sum(nums)) / nums.size();
  }

  // auto median(voi& nums) -> double {  
  //     ranges::sort(nums.begin(), nums.end());
  //     auto n = nums.size();
  //     return n % 2 == 0 ? (nums[n/2 - 1] + nums[n/2]) / 2.0 : nums[n/2];
  // }

  auto median(voi& nums) -> double {
      auto n = nums.size();
      auto mid = nums.begin() + n / 2;
      std::nth_element(nums.begin(), mid, nums.end());

      if (n % 2 == 0) {
          auto mid1 = std::max_element(nums.begin(), mid);
          return (*mid1 + *mid) / 2.0;
      } else {
          return *mid;
      }
  }
}
