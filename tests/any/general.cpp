#include "../../any.hpp"

#include <iostream>

int main() {
  std::cout << std::boolalpha;

  // any type
  gkxx::any a = 1;
  std::cout << a.type().name() << ": " << gkxx::any_cast<int>(a) << '\n';
  a = 3.14;
  std::cout << a.type().name() << ": " << gkxx::any_cast<double>(a) << '\n';
  a = true;
  std::cout << a.type().name() << ": " << gkxx::any_cast<bool>(a) << '\n';

  // bad cast
  try {
    a = 1;
    std::cout << gkxx::any_cast<float>(a) << '\n';
  } catch (const gkxx::bad_any_cast &e) {
    std::cout << e.what() << '\n';
  }

  // has value
  a = 2;
  if (a.has_value())
    std::cout << a.type().name() << ": " << gkxx::any_cast<int>(a) << '\n';

  // reset
  a.reset();
  if (!a.has_value())
    std::cout << "no value\n";

  // pointer to contained data
  a = 3;
  int *i = gkxx::any_cast<int>(&a);
  std::cout << *i << '\n';
}