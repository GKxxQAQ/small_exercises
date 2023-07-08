#include "../../meta_struct.hpp"
#include <string>
#include <iostream>

using namespace gkxx::meta_struct;

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
  Person p{init<"id"> = 1, init<"name"> = "John"};
  std::cout << get<"id">(p) << " " << get<"name">(p) << std::endl;
  p = Person{"name"_init = "Alice", "id"_init = 2};
  std::cout << get<"id">(p) << " " << get<"name">(p) << std::endl;

  using CopyMoveTest = meta_struct<member<"a", Test &>, member<"b", Test>>;
  Test t1{};
  Test t2{};

  CopyMoveTest cmt1{init<"a"> = t1, init<"b"> = t2};
  std::cout << "..." << std::endl;
  CopyMoveTest cmt2{init<"a"> = t1, init<"b"> = Test{}};

  return 0;
}