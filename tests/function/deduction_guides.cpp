#include "../../function.hpp"

int func(double) { return 0; }

int main() {
  gkxx::function f{func}; // guide #1 deduces function<int(double)>
  int i = 5;
  gkxx::function g = [&](double) { return i; }; // guide #2 deduces function<int(double)>
}