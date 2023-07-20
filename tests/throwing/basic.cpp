#include "../../throwing.hpp"
#include <stdexcept>

gkxx::throwing<int> integer_divide(int x, int y) {
  if (y == 0)
    co_yield std::overflow_error{"Divide by zero!"};
  else if (x % y != 0)
    co_yield std::range_error{"Result is not an integer!"};
  co_return x / y;
}

gkxx::throwing<bool> integer_divide_equals(int x, int y, int z) {
  if (co_await integer_divide(x, y) == z)
    co_return true;
  co_return false;
}

int main() {
  return gkxx::try_catch(
      [&]() -> gkxx::throwing<int> {
        if (co_await integer_divide_equals(4, 3, 2))
          co_return 1;
        else
          co_return 2;
      },
      [&](const std::overflow_error &) { return 3; },
      [&](const std::range_error &) { return 4; }, [&] { return 5; });
}