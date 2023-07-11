#ifndef GKXX_CONSTJSON_HPP
#define GKXX_CONSTJSON_HPP

#include <concepts>
#include <utility>

#include "fixed_string.hpp"
#include "switch_case.hpp"

/*
Tokens:
  integer: -?(0|[1-9][0-9]*)
  string: "[alpha/num, punctuations, escapes '\\', '\n', '\r', '\t', '\"']*"
  true, false, null
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
  inline constexpr bool is_digit(char c) {
    return c >= '0' && c <= '9';
  }
  inline constexpr bool is_supported_escape(char c) {
    return c == '\\' || c == 'n' || c == 'r' || c == 't' || c == '\"';
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
    struct result_t {
      using token = Token;
      static constexpr auto end_pos = EndPos;
    };

   private:
    static constexpr auto match_true() noexcept {
      if constexpr (Pos + 3 < Src.size() && Src[Pos + 1] == 'r' &&
                    Src[Pos + 2] == 'u' && Src[Pos + 3] == 'e')
        return result_t<ErrorToken<"expected 'true'">, Pos>{};
      else
        return result_t<True, Pos + 4>{};
    }
    static constexpr auto match_false() noexcept {
      if constexpr (Pos + 4 < Src.size() && Src[Pos + 1] == 'a' &&
                    Src[Pos + 2] == 'l' && Src[Pos + 3] == 's' &&
                    Src[Pos + 4] == 'e')
        return result_t<ErrorToken<"expected 'false'">, Pos>{};
      else
        return result_t<False, Pos + 5>{};
    }
    static constexpr auto match_null() noexcept {
      if constexpr (Pos + 3 < Src.size() && Src[Pos + 1] == 'u' &&
                    Src[Pos + 2] == 'l' && Src[Pos + 3] == 'l')
        return result_t<ErrorToken<"expected 'null'">, Pos>{};
      else
        return result_t<Null, Pos + 4>{};
    }

    struct string_matcher {
      static constexpr auto first_scan() noexcept {
        constexpr auto failure = Pos;
        if constexpr (Pos + 1 == Src.size())
          return std::pair{failure, 0};
        else {
          auto cur = Pos + 1;
          auto escape = 0;
          while (cur < Src.size() && Src[cur] != '"') {
            if (Src[cur] == '\\') {
              ++cur;
              if (cur < Src.size() && is_supported_escape(Src[cur])) {
                ++cur;
                ++escape;
              } else
                return std::pair{cur, -1};
            } else
              ++cur;
          }
          if (cur < Src.size() && Src[cur] == '"')
            return std::pair{cur, escape};
          else
            return std::pair{failure, 0};
        }
      }
      static constexpr auto get_contents() noexcept {
        char contents[end_quote_pos - Pos - 1 - escape_cnt];
        std::size_t fill = 0;
        for (auto i = Pos + 1; i != end_quote_pos; ++i) {
          if (Src[i] == '\\') {
            ++i;
            if (Src[i] == '\\')
              contents[fill++] = '\\';
            else if (Src[i] == 'n')
              contents[fill++] = '\n';
            else if (Src[i] == 'r')
              contents[fill++] = '\r';
            else if (Src[i] == 't')
              contents[fill++] = '\t';
            else // Src[i] == '\"'
              contents[fill++] = '\"';
          } else
            contents[fill++] = Src[i];
        }
        return fixed_string(contents);
      }
      static constexpr auto first_scan_result = first_scan();
      static constexpr auto end_quote_pos = first_scan_result.first;
      static constexpr auto escape_cnt = first_scan_result.second;
      static constexpr auto get_result() noexcept {
        if constexpr (end_quote_pos == Pos)
          return result_t<ErrorToken<"invalid string">, Pos>{};
        else if constexpr (escape_cnt == -1)
          return result_t<ErrorToken<"unsupported escape">, end_quote_pos>{};
        else
          return result_t<String<get_contents()>, end_quote_pos + 1>{};
      }
    };

    struct integer_matcher {
      template <std::size_t Cur>
      struct next_nondigit_pos {
        static constexpr auto result = Cur < Src.size() && is_digit(Src[Cur])
                                           ? next_nondigit_pos<Cur + 1>::result
                                           : Cur;
      };
      template <std::size_t start, std::size_t end>
      struct calc_value {
        static constexpr std::size_t
            result = end - start > 10
                         ? 0
                         : calc_value<start, end - 1>::result * 10u +
                               (Src[end - 1] - '0');
      };
      template <std::size_t pos>
      struct calc_value<pos, pos> {
        static constexpr std::size_t result = 0;
      };
      static constexpr auto neg = (Src[Pos] == '-');
      static constexpr auto start_pos = neg ? Pos + 1 : Pos;
      static constexpr auto end_pos = next_nondigit_pos<start_pos>::result;
      static constexpr auto digits_length = end_pos - start_pos;
      static constexpr auto too_many_leading_zeros =
          digits_length >= 1 && Src[start_pos] == '0' && digits_length >= 2;
      static constexpr std::size_t value =
          calc_value<start_pos, end_pos>::result;
      static constexpr auto overflow = value >
                                       (2147483647ul + static_cast<int>(neg));
      using result = typename meta::switch_<
          0,
          meta::case_if<[](...) { return digits_length == 0; },
                        result_t<ErrorToken<"expected integer">, start_pos>>,
          meta::case_if<[](...) { return (digits_length > 10); },
                        result_t<ErrorToken<"integer too long">, start_pos>>,
          meta::case_if<
              [](...) { return too_many_leading_zeros; },
              result_t<ErrorToken<"too many leading zeros">, start_pos>>,
          meta::case_if<[](...) { return overflow; },
                        result_t<ErrorToken<"integer value exceeding the range "
                                            "of 32-bit signed integers">,
                                 start_pos>>,
          meta::default_<result_t<
              Integer<neg ? -static_cast<int>(value) : static_cast<int>(value)>,
              end_pos>>>::type;
    };

   public:
    using result = typename meta::switch_<
        Src[Pos], meta::case_<'{', result_t<LBrace, Pos + 1>>,
        meta::case_<'}', result_t<RBrace, Pos + 1>>,
        meta::case_<'[', result_t<LBracket, Pos + 1>>,
        meta::case_<']', result_t<RBracket, Pos + 1>>,
        meta::case_<',', result_t<Comma, Pos + 1>>,
        meta::case_<':', result_t<Colon, Pos + 1>>,
        meta::case_<'t', decltype(match_true())>,
        meta::case_<'f', decltype(match_false())>,
        meta::case_<'n', decltype(match_null())>,
        meta::case_<'"', typename string_matcher::result>,
        meta::case_if<[](char c) { return c == '-' || is_digit(c); },
                      typename integer_matcher::result>,
        meta::default_<result_t<ErrorToken<"Unrecognized token">, Pos>>>::type;
  };

} // namespace tokenizer

} // namespace gkxx::constjson

#endif // GKXX_CONSTJSON_HPP