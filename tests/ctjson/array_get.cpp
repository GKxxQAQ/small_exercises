#include "../../ctjson.hpp"

#include <iostream>

int main() {
  using namespace gkxx::ctjson;
  using array =
      Array<Integer<42>, String<"hello">, Integer<0>, Array<>, Object<>>;
  std::cout << pretty_type_name<array::get<0>>() << std::endl;
  std::cout << pretty_type_name<array::get<1>>() << std::endl;
  std::cout << pretty_type_name<array::get<2>>() << std::endl;
  std::cout << pretty_type_name<array::get<3>>() << std::endl;
  std::cout << pretty_type_name<array::get<4>>() << std::endl;

  // The following should fail.
  // std::cout << pretty_type_name<array::get<5>>() << std::endl;

  return 0;
}