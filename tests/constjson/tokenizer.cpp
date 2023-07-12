#include "../../constjson.hpp"

#include "../../type_name.hpp"

#include <fstream>
#include <iostream>

constexpr const char cppconfig[] = R"(
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

constexpr const char tasks[] = R"(
  {
    "tasks": [
        {
            "type": "cppbuild",
            "label": "C/C++: g++-12 build active file",
            "command": "/usr/bin/g++-12",
            "args": [
                "-fdiagnostics-color=always",
                "-g",
                "${file}",
                "-o",
                "${fileDirname}/${fileBasenameNoExtension}",
                "-std=c++20"
            ],
            "options": {
                "cwd": "${fileDirname}"
            },
            "problemMatcher": [
                "$gcc"
            ],
            "group": {
                "kind": "build",
                "isDefault": true
            },
            "detail": "Task generated by Debugger."
        }
    ],
    "version": "2.0.0"
}
)";

int main() {
  using type =
      gkxx::constjson::Tokenizer<"42 \"hello world\" : , , :::">::result;
  std::cout << gkxx::get_type_name<type>() << std::endl;
  using big_result = gkxx::constjson::Tokenizer<cppconfig>::result;
  std::cout << gkxx::get_type_name<big_result>() << std::endl;
  std::ofstream("cppconfig.json")
      << big_result::reconstruct_string() << std::endl;
  using tasks_result = gkxx::constjson::Tokenizer<tasks>::result;
  std::cout << gkxx::get_type_name<tasks_result>() << std::endl;
  std::ofstream("tasks.json")
      << tasks_result::reconstruct_string() << std::endl;
  return 0;
}