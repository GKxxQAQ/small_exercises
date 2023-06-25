#include "../../any.hpp"

#include <iostream>
#include <string>
#include <type_traits>
#include <utility>

int main() {
  // Simple example

  auto a1 = gkxx::any(12);

  std::cout << "1) a1 is int: " << gkxx::any_cast<int>(a1) << '\n';

  try {
    auto s = gkxx::any_cast<std::string>(a1); // throws
  } catch (const gkxx::bad_any_cast &e) {
    std::cout << "2) " << e.what() << '\n';
  }

  // Pointer example

  if (int *i = gkxx::any_cast<int>(&a1))
    std::cout << "3) a1 is int: " << *i << '\n';
  else if (std::string *s = gkxx::any_cast<std::string>(&a1))
    std::cout << "3) a1 is std::string: " << *s << '\n';
  else
    std::cout << "3) a1 is another type or unset\n";

  // Advanced example

  a1 = std::string("hello");

  auto &ra = gkxx::any_cast<std::string &>(a1); //< reference
  ra[1] = 'o';

  std::cout << "4) a1 is string: " << gkxx::any_cast<std::string const &>(a1)
            << '\n'; //< const reference

  auto s1 = gkxx::any_cast<std::string &&>(std::move(a1)); //< rvalue reference

  // Note: s1 is a move-constructed std::string:
  static_assert(std::is_same_v<decltype(s1), std::string>);

  // Note: the std::string in a1 is left in valid but unspecified state
  std::cout << "5) a1.size(): "
            << gkxx::any_cast<std::string>(&a1)->size() //< pointer
            << '\n';

  std::cout << "6) s1: " << s1 << '\n';
}