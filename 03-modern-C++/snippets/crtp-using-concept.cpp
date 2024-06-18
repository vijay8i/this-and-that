#include <cmath>
#include <concepts>
#include <vector>
#include <print>  // C++23, not available in g++ use clang++ >= 17

#include "describe.hh"

// define the concept of a 2d point
template<typename T>
concept point2d = requires(T p) {
  // require that argument has `x`, and `y` fields
  p.x;
  p.y;

  // require that `x` is a number
  requires std::is_arithmetic_v<decltype(p.x)>;
  // and that `y` is the same type as `x`
  requires std::same_as<decltype(p.x), decltype(p.y)>;
  // and that type `T` is composed only of `x` and `y`
  requires sizeof(p) == sizeof(p.x) * 2;
  
  // require that construction of `T` from `x` and `y` is a valid expression
  T{ p.x, p.y };
};

// define the concept of a 3d ptor; question: can we use inheritance?
template<typename T>
concept point3d = requires(T p) {
  p.x;
  p.y;
  p.z;
  requires std::is_arithmetic_v<decltype(p.x)>;
  requires std::same_as<decltype(p.x), decltype(p.y)>;
  requires std::same_as<decltype(p.x), decltype(p.z)>;
  requires sizeof(p) == sizeof(p.x) * 3;
  T{ p.x, p.y, p.z };
};

// define the concept of a ptor to be one of the concepts above
template<typename T>
concept point_interface = point2d<T> || point3d<T>;

// default to false
template<point_interface T>
inline constexpr bool is_point = false;

// define concretely what a `ptor` is in terms of the interface
template<typename T>
concept point = is_point<T>;

// use the concept of the ptor
template<point Point>
auto operator+(const Point& self, const Point& other) noexcept {
  if constexpr(point3d<Point>) {
    return Point {self.x + other.x, self.y + other.y, self.z + other.z };
  } else {
    return Point {self.x + other.x, self.y + other.y };
  }
}

auto norm(const point auto& self) noexcept {
  auto norm_sq = self.x * self.x + self.y * self.y;
  if constexpr(point3d<decltype(self)>) {
    norm_sq += self.z + self.z;
  }
  return std::sqrt(norm_sq);
}

struct Point2f {
  float x;
  float y;
};

struct Point3i {
  int x;
  int y;
  int z;
};

template<> inline constexpr bool is_point<Point2f> = true;
template<> inline constexpr bool is_point<Point3i> = true;

// this will error, because of line 27; but unlike the template errors
// that stream pages of misery, the error is very much easy to follow
// and troubleshoot
// struct Point2_err {
//   double x;
//   float y;
// };
// template<> inline constexpr bool is_pointr<Point2_err> = true;

int main() {
  using namespace std;
  vector foo = vector{
    Point3i{7, 8}
  };
  auto fe = foo[0];
  println("foo is: {}", describe<decltype(foo)>());
  println("foo[0] is: {}", describe<decltype(fe)>());
  // println("foo is: {}", describe(foo));
  // println("foo[0] is: {}", describe(fe));

  // cout << "Hello, world!\n";
  // println("Hello, world!");

    // Point2_err v1{1, 0};
    // Point2_err v2{0, 1};
    // Compiles fine up until here

    // v1 + v2; // Error on instantiation, like CRTP!
}
