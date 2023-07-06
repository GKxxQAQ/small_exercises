#include "../../meta_struct.hpp"

#include <string>

using namespace gkxx;

int main() {
  using Person = meta_struct<
      member<"id", int, required>,
      member<"score", int, [](auto &self) { return get<"id">(self) + 1; }>,
      member<"name", std::string>>;
  Person p{arg<"id"> = 2, arg<"name"> = "John"};
}