# C++ Concepts

## [Emulating CRTP with C++ concepts](https://joelfilho.com/blog/2021/emulating_crtp_with_cpp_concepts/)
The earlier we can catch errors, the better; C++ concepts help us do
that. A bonus of using concepts is that we get much better compiler
errors that without them.

See also:
- https://www.cppstories.com/2018/03/ifconstexpr/
- https://www.cppstories.com/2016/02/notes-on-c-sfinae/

```C++
#include <cmath>
#include <concepts>
#include <vector>
#include <print>  // C++23, not available in g++ use clang++ >= 17
  
// define the concept of a 2d vector (as in math, not STL)
template<typename T>
concept vector2d = requires(T vec) {
  // require that argument has `x`, and `y` fields
  vec.x;
  vec.y;

  // require that `x` is a number
  requires std::is_arithmetic_v<decltype(vec.x)>;
  // and that `y` is the same type as `x`
  requires std::same_as<decltype(vec.x), decltype(vec.y)>;
  // and that type `T` is composed only of `x` and `y`
  requires sizeof(vec) == sizeof(vec.x) * 2;
  
  // require that construction of `T` from `x` and `y` is a valid expression
  T{ vec.x, vec.y };
};

// define the concept of a 3d vector; question: can we use inheritance?
template<typename T>
concept vector3d = requires(T vec) {
  vec.x;
  vec.y;
  vec.z;
  requires std::is_arithmetic_v<decltype(vec.x)>;
  requires std::same_as<decltype(vec.x), decltype(vec.y)>;
  requires std::same_as<decltype(vec.x), decltype(vec.z)>;
  requires sizeof(vec) == sizeof(vec.x) * 3;
  T{ vec.x, vec.y, vec.z };
};

// define the concept of a vector to be one of the concepts above
template<typename T>
concept vector_interface = vector2d<T> || vector3d<T>;

// default to false
template<vector_interface T>
inline constexpr bool is_vector = false;

// define concretely what a `vector` is in terms of the interface
template<typename T>
concept vector = is_vector<T>;

// use the concept of the vector
template<vector Vector>
auto operator+(const Vector& self, const Vector& other) noexcept {
  if constexpr(vector3d<Vector>) {
    return Vector {self.x + other.x, self.y + other.y, self.z + other.z };
  } else {
    return Vector {self.x + other.x, self.y + other.y };
  }
}

auto norm(const vector auto& self) noexcept {
  auto norm_sq = self.x * self.x + self.y * self.y;
  if constexpr(vector3d<decltype(self)>) {
    norm_sq += self.z + self.z;
  }
  return std::sqrt(norm_sq);
}

struct Vector2f {
  float x;
  float y;
};

struct Vector3i {
  int x;
  int y;
  int z;
};

template<> inline constexpr bool is_vector<Vector2f> = true;
template<> inline constexpr bool is_vector<Vector3i> = true;

// this will error, because of line 27; remove the comment and compile
// to see the error
// struct Vector2_err {
//   double x;
//   float y;
// };
// template<> inline constexpr bool is_vector<Vector2_err> = true;

auto main(){
  // std::vector foo = std::vector{{5,6}};
  // println("2d vector: {}", foo);

    Vector2_err v1{1, 0};
    Vector2_err v2{0, 1};
    // Compiles fine up until here

    v1 + v2; // Error on instantiation, like CRTP!
}

```