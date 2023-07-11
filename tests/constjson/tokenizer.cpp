#include "../../constjson.hpp"

#include "../../type_name.hpp"

#include <iostream>
#include <fstream>

constexpr const char big[] = R"(
  {
  "configuration": {
    "name": "Linux",
    "intelliSenseMode": "linux-clang-x64",
    "compilerPath": "/usr/bin/clang++-16",
    "cStandard": "c17",
    "cppStandard": "c++20",
    "includePath": [
      "/usr/local/boost_1_80_0/",
      "/home/gkxx/exercises/small_exercises/"
    ],
    "compilerArgs": [
      "-Wall",
      "-Wpedantic",
      "-Wextra"
    ]
  },
  "version": 4
}
)";

int main() {
  using type = gkxx::constjson::tokenizer::Tokenizer<
      "42 \"hello world\" : , , :::">::result;
  std::cout << gkxx::get_type_name<type>() << std::endl;
  using big_result = gkxx::constjson::tokenizer::Tokenizer<big>::result;
  std::cout << gkxx::get_type_name<big_result>() << std::endl;
  std::ofstream("output.json") << big_result::reconstruct_string() << std::endl;
  return 0;
}