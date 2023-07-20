#include "../../generator.hpp"
#include <vector>
#include <iostream>

gkxx::Generator<int> interleaved(std::vector<int> const &a,
                                 std::vector<int> const &b) {
  auto gen = [](auto const &v) -> gkxx::Generator<int> {
    for (auto const &e : v)
      co_yield e;
  };
  auto x = gen(a);
  auto y = gen(b);
  auto it1 = x.begin(), it2 = y.begin();
  auto end1 = x.end(), end2 = y.end();
  while (it1 != end1 || it2 != end2) {
    if (it1 != end1) {
      co_yield *it1;
      ++it1;
    }
    if (it2 != end2) {
      co_yield *it2;
      ++it2;
    }
  }
}

int main() {
  std::vector a{1, 3, 5, 7, 9, 11}, b{2, 4, 6, 8};
  for (auto const &x : interleaved(a, b))
    std::cout << x << ' ';
  std::cout << '\n';
  return 0;
}