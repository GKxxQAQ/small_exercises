#include "../../meta_struct.hpp"
#include <string>
#include <iostream>

using namespace gkxx;

using Person = meta_struct<member<"id", int>, member<"name", std::string>>;

struct Test {
  Test() = default;
  Test(const Test &) {
    std::cout << "Test::Test(const Test &)" << std::endl;
  }
  Test(Test &&) noexcept {
    std::cout << "Test::Test(Test &&)" << std::endl;
  }
  Test &operator=(const Test &) {
    std::cout << "Test::operator=(const Test &)" << std::endl;
    return *this;
  }
  Test &operator=(Test &&) noexcept {
    std::cout << "Test::operator=(Test &&)" << std::endl;
    return *this;
  }
};

int main() {
  Person p{arg<"id"> = 1, arg<"name"> = "John"};
  std::cout << get<"id">(p) << " " << get<"name">(p) << std::endl;
  p = Person{arg<"name"> = "Alice", arg<"id"> = 2};
  std::cout << get<"id">(p) << " " << get<"name">(p) << std::endl;

  using CopyMoveTest = meta_struct<member<"a", Test &>, member<"b", Test>>;
  Test t1{};
  Test t2{};

  CopyMoveTest cmt1{arg<"a"> = t1, arg<"b"> = t2};
  std::cout << "..." << std::endl;
  CopyMoveTest cmt2{arg<"a"> = t1, arg<"b"> = Test{}};

  return 0;
}