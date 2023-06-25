#include "../../any.hpp"

#include <functional>
#include <iomanip>
#include <iostream>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <vector>

template <class T, class F>
inline std::pair<const std::type_index, std::function<void(gkxx::any const &)>>
to_any_visitor(F const &f) {
  return {std::type_index(typeid(T)), [g = f](gkxx::any const &a) {
            if constexpr (std::is_void_v<T>)
              g();
            else
              g(gkxx::any_cast<T const &>(a));
          }};
}

static std::unordered_map<std::type_index,
                          std::function<void(gkxx::any const &)>>
    any_visitor{
        to_any_visitor<void>([] { std::cout << "{}"; }),
        to_any_visitor<int>([](int x) { std::cout << x; }),
        to_any_visitor<unsigned>([](unsigned x) { std::cout << x; }),
        to_any_visitor<float>([](float x) { std::cout << x; }),
        to_any_visitor<double>([](double x) { std::cout << x; }),
        to_any_visitor<char const *>(
            [](char const *s) { std::cout << std::quoted(s); }),
        // ... add more handlers for your types ...
    };

inline void process(const gkxx::any &a) {
  if (const auto it = any_visitor.find(std::type_index(a.type()));
      it != any_visitor.cend())
    it->second(a);
  else
    std::cout << "Unregistered type " << std::quoted(a.type().name());
}

template <class T, class F>
inline void register_any_visitor(F const &f) {
  std::cout << "Register visitor for type " << std::quoted(typeid(T).name())
            << '\n';
  any_visitor.insert(to_any_visitor<T>(f));
}

int main() {
  std::vector<gkxx::any> va{
      {}, 42, 123u, 3.14159f, 2.71828, "C++17",
  };

  std::cout << "{ ";
  for (const gkxx::any &a : va) {
    process(a);
    std::cout << ", ";
  }
  std::cout << "}\n";

  process(gkxx::any(0xFULL)); //< Unregistered type "y" (unsigned long long)
  std::cout << '\n';

  register_any_visitor<unsigned long long>(
      [](auto x) { std::cout << std::hex << std::showbase << x; });

  process(gkxx::any(0xFULL)); //< OK: 0xf
  std::cout << '\n';
}