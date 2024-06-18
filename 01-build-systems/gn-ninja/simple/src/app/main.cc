#include "simple-stats.hh"
#include <format> // C++20
#include <print>  // C++23, not available in g++; use clang++

auto main() -> int {
  using namespace ss;
  using namespace std;
  voi nums = {1, 2, 3, 4, 5};
  println("Sum: {}", sum(nums));
  println("Average: {:.2f}", average(nums));
  println("Median: {:.2f}", median(nums));
}