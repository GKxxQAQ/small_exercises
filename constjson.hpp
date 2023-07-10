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
  struct Integer {};

  template <typename T>
  inline constexpr auto is_integer_token = false;
  template <int N>
  inline constexpr auto is_integer_token<Integer<N>> = true;

  template <fixed_string S>
  struct String {};

  template <typename T>
  inline constexpr auto is_string_token = false;
  template <fixed_string S>
  inline constexpr auto is_string_token<String<S>> = true;

  struct True {};
  struct False {};
  struct Null {};

  struct LBrace {};
  struct RBrace {};
  struct LBracket {};
  struct RBracket {};
  struct Comma {};
  struct Colon {};

  template <fixed_string Msg>
  struct ErrorToken {};

  template <typename T>
  inline constexpr auto is_error_token = false;
  template <fixed_string Msg>
  inline constexpr auto is_error_token<ErrorToken<Msg>> = true;

  template <typename T, typename... Types>
  concept same_as_any = (... || std::same_as<T, Types>);

  template <typename T>
  concept CToken =
      same_as_any<T, True, False, Null, LBrace, RBrace, LBracket, RBracket,
                  Comma, Colon> ||
      is_integer_token<T> || is_string_token<T> || is_error_token<T>;

  template <CToken... Tokens>
  struct TokenSequence {};

  inline constexpr bool is_whitespace(char c) {
    return c == ' ' || c == '\n' || c == '\t' || c == '\r';
  }

  template <fixed_string Src>
  struct Tokenizer {
    template <std::size_t Pos>
    struct next_nonwhitespace_pos;

    template <std::size_t Pos>
    struct token_getter;

    template <std::size_t Pos, CToken... CurrentTokens>
    struct parser {
      // Src[Pos] is non-whitespace
      using token_getter = typename token_getter<Pos>::result;
      using new_token = typename token_getter::token;
      static constexpr auto next_pos =
          next_nonwhitespace_pos<token_getter::end_pos>::result;
      using result =
          typename parser<next_pos, CurrentTokens..., new_token>::result;
    };
    template <CToken... Tokens>
    struct parser<Src.size(), Tokens...> {
      using result = TokenSequence<Tokens...>;
    };

    using result = parser<next_nonwhitespace_pos<0>::result>::result;
  };

  template <fixed_string Src>
  template <std::size_t Pos>
  struct Tokenizer<Src>::next_nonwhitespace_pos {
    static constexpr auto move() noexcept {
      auto i = Pos;
      while (i < Src.size() && is_whitespace(Src[i]))
        ++i;
      return i;
    }
    static constexpr auto result = move();
  };

  template <fixed_string Src>
  template <std::size_t Pos>
  struct Tokenizer<Src>::token_getter {
    static_assert(!is_whitespace(Src[Pos]),
                  "token_getter encounters a whitespace");
    template <CToken Token, std::size_t EndPos>
    struct result_wrapper {
      using token = Token;
      static constexpr auto end_pos = EndPos;
    };

   private:
    static constexpr auto match_true() noexcept {
      if constexpr (Pos + 3 < Src.size() && Src[Pos + 1] == 'r' &&
                    Src[Pos + 2] == 'u' && Src[Pos + 3] == 'e')
        return result_wrapper<ErrorToken<"expected 'true'">, Pos>{};
      else
        return result_wrapper<True, Pos + 4>{};
    }
    static constexpr auto match_false() noexcept {
      if constexpr (Pos + 4 < Src.size() && Src[Pos + 1] == 'a' &&
                    Src[Pos + 2] == 'l' && Src[Pos + 3] == 's' &&
                    Src[Pos + 4] == 'e')
        return result_wrapper<ErrorToken<"expected 'false'">, Pos>{};
      else
        return result_wrapper<False, Pos + 5>{};
    }
    static constexpr auto match_null() noexcept {
      if constexpr (Pos + 3 < Src.size() && Src[Pos + 1] == 'u' &&
                    Src[Pos + 2] == 'l' && Src[Pos + 3] == 'l')
        return result_wrapper<ErrorToken<"expected 'null'">, Pos>{};
      else
        return result_wrapper<Null, Pos + 4>{};
    }
    static constexpr auto match_string() noexcept {
      // TODO: match_string
    }
    static constexpr auto match_integer() noexcept {
      // TODO: match_integer
    }
    static constexpr auto get() noexcept {
      if constexpr (Src[Pos] == '{')
        return result_wrapper<LBrace, Pos + 1>{};
      else if constexpr (Src[Pos] == '}')
        return result_wrapper<RBrace, Pos + 1>{};
      else if constexpr (Src[Pos] == '[')
        return result_wrapper<LBracket, Pos + 1>{};
      else if constexpr (Src[Pos] == ']')
        return result_wrapper<RBracket, Pos + 1>{};
      else if constexpr (Src[Pos] == ',')
        return result_wrapper<Comma, Pos + 1>{};
      else if constexpr (Src[Pos] == ':')
        return result_wrapper<Colon, Pos + 1>{};
      else if constexpr (Src[Pos] == 't')
        return match_true();
      else if constexpr (Src[Pos] == 'f')
        return match_false();
      else if constexpr (Src[Pos] == 'n')
        return match_null();
      else if constexpr (Src[Pos] == '"')
        return match_string();
      else
        return match_integer();
    }

   public:
    using result = decltype(get());
  };

} // namespace tokenizer

} // namespace gkxx::constjson

#endif // GKXX_CONSTJSON_HPP