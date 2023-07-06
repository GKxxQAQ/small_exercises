#include "../../meta_struct.hpp"

#include <iostream>
#include <string>

using namespace gkxx;

using Person = meta_struct<
    member<"id", int>,
    member<"score", int, [](auto &self) { return get<"id">(self) + 1; }>,
    member<"name", std::string, [] { return "John"; }>>;

int main() {
  Person p;
  std::cout << get<"id">(p) << " " << get<"name">(p) << " " << get<"score">(p) << std::endl;
  return 0;
}