#include "../../constjson.hpp"
#include "../../type_name.hpp"
#include <iostream>

int main() {
  using namespace gkxx::constjson;

  using result = parse<"{\"a\": 1, \"b\": 2, \"a\": 3}">::result;
  std::cout << gkxx::get_type_name<result>() << std::endl;

  // The following should fail.
  // using X = Object<Member<"a", Integer<1>>, Member<"b", Integer<2>>,
                  //  Member<"a", Integer<3>>>;

  using Y [[maybe_unused]] = Object<>;
  using Z [[maybe_unused]] = Object<Member<"a", Integer<1>>>;
}