#include "../../generator.hpp"
#include <iostream>
#include <ranges>
#include <string>
#include <tuple>
#include <vector>

using namespace std::string_literals;
namespace ranges = std::ranges;

template <ranges::input_range Rng1, ranges::input_range Rng2>
gkxx::Generator<std::tuple<ranges::range_reference_t<Rng1>,
                           ranges::range_reference_t<Rng2>>>
zip(Rng1 r1, Rng2 r2) {
  auto it1 = ranges::begin(r1);
  auto it2 = ranges::begin(r2);
  auto end1 = ranges::end(r1);
  auto end2 = ranges::end(r2);
  for (; it1 != end1 && it2 != end2; ++it1, ++it2)
    co_yield {*it1, *it2};
}

int main() {
  std::vector v{1, 2, 3, 4, 5};
  std::vector vs{"abc"s, "acd"s, "abd"s};
  for (auto &&[i, s] : zip(v, vs))
    std::cout << "(" << i << ", " << s << ")\n";
  return 0;
}