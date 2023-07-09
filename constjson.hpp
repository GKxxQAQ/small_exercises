#ifndef GKXX_CONSTJSON_HPP
#define GKXX_CONSTJSON_HPP

#include <concepts>

#include "fixed_string.hpp"

/*
Tokens:
  integer: -?(0|[1-9][0-9]*)
  string: "[alphabets, digits, punctuations, escapes '\\', '\n', '\r', '\t',
'\"']*" true, false, null
  '{', '}', '[', ']', ',', ':'
 */

/*
json -> {element}
element -> {ws} {value} {ws}
value -> {object}
       | {array}
       | {string}
       | {integer}
       | true
       | false
       | null
object -> \{ {ws} \}
        | \{ {members} \}
members -> {member}
         | {member} , {members}
member -> {ws} {string} {ws} : {element}
array -> [{ws}]
       | [{elements}]
elements -> {element}
          | {element} , {elements}
 */

/*
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
      "-Wextra",
    ]
  },
  "version": 4
}

Object<
  Member<"configuration", Object<
    Member<"name", String<"Linux">>,
    Member<"IntelliSenseMode", String<"linux-clang-x64">>,
    Member<"compilerPath", String<"/usr/bin/clang++-16">>,
    Member<"cStandard", String<"c17">>,
    Member<"cppStandard", String<"c++20">>,
    Member<"includePath", Array<
      String<"/usr/local/boost_1_80_0/">,
      String<"/home/gkxx/exercises/small_exercises/">
    >>,
    Member<"compilerArgs", Array<
      String<"-Wall">,
      String<"-Wpedantic">,
      String<"-Wextra">
    >>
  >>,
  Member<"version", Int<4>>
>
 */

namespace gkxx::constjson {

namespace tokenizer {

  template <int N>
  struct Integer {
    using integer_tag = Integer<N>;
  };

  template <fixed_string S>
  struct String {
    using string_tag = String<S>;
  };

  struct True {};
  struct False {};
  struct Null {};

  struct LBrace {};
  struct RBrace {};
  struct LBracket {};
  struct RBracket {};
  struct Comma {};
  struct Colon {};

  template <typename T, typename... Types>
  concept same_as_any = (... || std::same_as<T, Types>);

  template <typename T>
  concept CToken = same_as_any<T, True, False, Null, LBrace, RBrace, LBracket,
                               RBracket, Comma, Colon> ||
                   requires { typename T::integer_tag; } ||
                   requires { typename T::string_tag; };

  template <CToken... Tokens>
  struct TokenSequence {};

} // namespace tokenizer

} // namespace gkxx::constjson

#endif // GKXX_CONSTJSON_HPP