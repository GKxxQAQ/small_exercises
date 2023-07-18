#include "../../constjson.hpp"
#include <iostream>
#include <fstream>

int main() {
  using namespace gkxx::constjson;
  using cppconfig = Object<
      Member<
          "configuration",
          Object<Member<"name", String<"Linux">>,
                 Member<"intelliSenseMode", String<"linux-clang-x64">>,
                 Member<"compilerPath", String<"/usr/bin/clang++-16">>,
                 Member<"cStandard", String<"c17">>,
                 Member<"cppStandard", String<"c++20">>,
                 Member<"includePath",
                        Array<String<"/usr/local/boost_1_80_0/">,
                              String<"/home/gkxx/exercises/small_exercises/">>>,
                 Member<"compilerArgs",
                        Array<String<"-Wall">, String<"-Wpedantic">,
                              String<"-Wextra">>>>>,
      Member<"version", Integer<4>>>;
  std::ofstream("cppconfig.json") << cppconfig::to_string() << std::endl;
  return 0;
}