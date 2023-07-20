#include "../../generator.hpp"
#include <coroutine>
#include <iostream>

struct Widget {
  unsigned value;
  Widget(unsigned v) : value{v} {}
  Widget(Widget const &other) : value{other.value} {
    std::cout << "Widget::Widget(Widget const &) called.\n";
  }
  Widget &operator=(const Widget &other) {
    std::cout << "Widget &Widget::operator=(const Widget &) called.\n";
    value = other.value;
    return *this;
  }
  Widget(Widget &&other) noexcept : value{other.value} {
    std::cout << "Widget::Widget(Widget &&) called.\n";
  }
  Widget &operator=(Widget &&other) noexcept {
    std::cout << "Widget &Widget::operator=(Widget &&) called.\n";
    value = other.value;
    return *this;
  }
  ~Widget() = default;
};

gkxx::Generator<Widget> range(unsigned n) {
  for (unsigned i{}; i != n; ++i)
    co_yield i;
}

int main() {
  auto r = range(10);
  for (auto &[i] : r)
    std::cout << i << ' ';
  std::cout << std::endl;
  return 0;
}