#include "../../meta_struct.hpp"

#include <string>
#include <iostream>

int main() {
  using namespace gkxx::meta_struct;
  using Person = meta_struct<member<"id", int>, member<"name", std::string>>;
  Person p;
  get<"id">(p) = 1;
  get<"name">(p) = "John";
  std::cout << get<"id">(p) << " " << get<"name">(p) << std::endl;
  return 0;
}