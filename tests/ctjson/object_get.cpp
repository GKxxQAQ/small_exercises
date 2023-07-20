#include "../../ctjson.hpp"
#include <fstream>
#include <iostream>

int main() {
  using namespace gkxx::ctjson;
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
  using config = cppconfig::get<"configuration">;
  std::cout << pretty_type_name<config>() << std::endl;
  using name = config::get<"name">;
  std::cout << pretty_type_name<name>() << std::endl;
  using intelmode = config::get<"intelliSenseMode">;
  std::cout << pretty_type_name<intelmode>() << std::endl;
  using includepath = config::get<"includePath">;
  std::cout << pretty_type_name<includepath>() << std::endl;

  // The following should fail.
  // using nat = cppconfig::get<"hello">;
  return 0;
}