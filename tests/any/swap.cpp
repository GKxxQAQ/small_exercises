#include "../../any.hpp"

#include <iostream>
#include <string>

int main() {
  using namespace std::string_literals;
  gkxx::any a = 1, b = "hello"s;
  std::swap(a, b);
  std::cout << a.type().name() << ", " << b.type().name() << std::endl;
  return 0;
}