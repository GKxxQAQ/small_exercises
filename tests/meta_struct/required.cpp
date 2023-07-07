#include "../../meta_struct.hpp"

#include <string>
#include <iostream>

using namespace gkxx;

int main() {
  using Person = meta_struct<
      member<"id", int, required>,
      member<"score", int, [](auto &self) { return get<"id">(self) + 1; }>,
      member<"name", std::string, required>>;
  Person p{arg<"id"> = 2, arg<"name"> = "Alice"};
  std::cout << get<"id">(p) << ' ' << get<"score">(p) << ' ' << get<"name">(p)
            << std::endl;
}