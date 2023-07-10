#include "../../switch_case.hpp"

using namespace gkxx::meta;

template <int N>
struct Foo {
  using type = typename switch_<N,
      case_<0, int>,
      case_<1, double>,
      case_<2, char *>
      default_<void>
  >::type;
};

int main() {
  static_assert(std::is_same_v<Foo<0>::type, int>);
  static_assert(std::is_same_v<Foo<1>::type, double>);
  static_assert(std::is_same_v<Foo<2>::type, char *>);
  static_assert(std::is_same_v<Foo<3>::type, void>);
}