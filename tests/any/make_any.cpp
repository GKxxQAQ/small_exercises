#include "../../any.hpp"

#include <complex>
#include <functional>
#include <iostream>
#include <string>

int main() {
  auto a0 = gkxx::make_any<std::string>("Hello, gkxx::any!\n");
  auto a1 = gkxx::make_any<std::complex<double>>(0.1, 2.3);

  std::cout << gkxx::any_cast<std::string &>(a0);
  std::cout << gkxx::any_cast<std::complex<double> &>(a1) << '\n';

  using lambda = std::function<void(void)>;

  // Put a lambda into gkxx::any. Attempt #1 (failed).
  gkxx::any a2 = [] { std::cout << "Lambda #1.\n"; };
  std::cout << "a2.type() = \"" << a2.type().name() << "\"\n";

  // any_cast casts to <void(void)> but actual type is not
  // a std::function..., but ~ main::{lambda()#1}, and it is
  // unique for each lambda. So, this throws...
  try {
    gkxx::any_cast<lambda>(a2)();
  } catch (gkxx::bad_any_cast const &ex) {
    std::cout << ex.what() << '\n';
  }

  // Put a lambda into gkxx::any. Attempt #2 (successful).
  auto a3 = gkxx::make_any<lambda>([] { std::cout << "Lambda #2.\n"; });
  std::cout << "a3.type() = \"" << a3.type().name() << "\"\n";
  gkxx::any_cast<lambda>(a3)();
}