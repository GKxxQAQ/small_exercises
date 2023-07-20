#include "../../generator.hpp"
#include <coroutine>
#include <iostream>
#include <utility>

gkxx::Generator<unsigned> fibonacci(unsigned n) {
  co_yield 0u;
  unsigned a = 1, b = 1;
  while (n--) {
    co_yield a;
    a = std::exchange(b, a + b);
  }
}

int main() {
  for (auto i : fibonacci(10))
    std::cout << i << '\n';
  std::cout << std::flush;
  return 0;
}