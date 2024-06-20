#include "simple-stats.hh"
#include <print>  // C++23, not available in g++; use clang++17 or higher

auto main() -> int {
  using namespace std;
  using namespace ss;

  voi nums = {1, 2, 3, 4, 5};
  println("Sum: {}", sum(nums));
  println("Average: {:.2f}", average(nums));
  println("Median: {:.2f}", median(nums));
}