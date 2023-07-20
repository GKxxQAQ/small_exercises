#include "../../generator.hpp"

#include <iostream>

gkxx::Generator<int> range(int n) {
  for (int i = 0; i != n; ++i)
    co_yield i;
}

int main() {
  for (auto i : range(10))
    std::cout << i << std::endl;
  return 0;
}