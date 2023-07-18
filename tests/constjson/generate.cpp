#include "../../constjson.hpp"
#include <fstream>
#include <iostream>

int main() {
  using namespace gkxx::constjson;
  using cppconfig = Object<
      Member<"configuration",
             Object<Member<"name", String<"Linux">>,
                    Member<"intelliSenseMode", String<"linux-clang-x64">>,
                    Member<"compilerPath", String<"/usr/bin/clang++-16">>,
                    Member<"cStandard", String<"c17">>,
                    Member<"cppStandard", String<"c++20">>,
                    Member<"includePath",
                           ArrayStr<"/usr/local/boost_1_80_0/",
                                    "/home/gkxx/exercises/small_exercises/">>,
                    Member<"compilerArgs",
                           ArrayStr<"-Wall", "-Wpedantic", "-Wextra">>>>,
      Member<"version", Integer<4>>>;
  std::ofstream("cppconfig.json") << cppconfig::to_string() << std::endl;
  return 0;
}