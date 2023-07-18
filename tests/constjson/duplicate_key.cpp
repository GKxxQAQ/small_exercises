#include "../../constjson.hpp"
#include "../../type_name.hpp"
#include <iostream>

int main() {
  using namespace gkxx::constjson;

  using result = parse<"{\"a\": 1, \"b\": 2, \"a\": 3}">::result;
  std::cout << gkxx::get_type_name<result>() << std::endl;
}