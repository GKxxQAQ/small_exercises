#ifndef GKXX_CONSTJSON_HPP
#define GKXX_CONSTJSON_HPP

#include <concepts>
#include <string>
#include <utility>
#include <tuple>

#include "fixed_string.hpp"
#include "is_specialization_of.hpp"
#include "switch_case.hpp"

/*
Tokens:
  integer: -?(0|[1-9][0-9]*)
  string: "[alpha/num, punctuations, escapes '\\', '\n', '\r', '\t', '\"']*"
  true, false, null
  '{', '}', '[', ']', ',', ':'
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
      "-Wextra"
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

template <int N>
struct Integer {
  static constexpr int value = N;
};

template <typename T>
inline constexpr auto is_integer_token = false;
template <int N>
inline constexpr auto is_integer_token<Integer<N>> = true;

template <fixed_string S>
struct String {
  static constexpr fixed_string value = S;
};

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

template <fixed_string Msg, std::size_t Pos>
struct ErrorToken {
  static constexpr fixed_string message = Msg;
  static constexpr std::size_t position = Pos;
};

template <typename T>
inline constexpr auto is_error_token = false;
template <fixed_string Msg, std::size_t Pos>
inline constexpr auto is_error_token<ErrorToken<Msg, Pos>> = true;

template <typename T, typename... Types>
concept same_as_any = (... || std::same_as<T, Types>);

template <typename T>
concept CToken = same_as_any<T, True, False, Null, LBrace, RBrace, LBracket,
                             RBracket, Comma, Colon> ||
                 is_integer_token<T> || is_string_token<T> || is_error_token<T>;

template <CToken... Tokens>
struct TokenSequence {
  static constexpr std::string reconstruct_string() {
    auto to_string = []<typename T>(T *) -> std::string {
      if constexpr (is_integer_token<T>)
        return std::to_string(T::value);
      else if constexpr (is_string_token<T>)
        return "\"" + T::value.as_string() + "\"";
      else if constexpr (std::is_same_v<T, True>)
        return "true";
      else if constexpr (std::is_same_v<T, False>)
        return "false";
      else if constexpr (std::is_same_v<T, Null>)
        return "null";
      else if constexpr (std::is_same_v<T, LBrace>)
        return "{";
      else if constexpr (std::is_same_v<T, RBrace>)
        return "}";
      else if constexpr (std::is_same_v<T, LBracket>)
        return "[";
      else if constexpr (std::is_same_v<T, RBracket>)
        return "]";
      else if constexpr (std::is_same_v<T, Comma>)
        return ",";
      else if constexpr (std::is_same_v<T, Colon>)
        return ":";
      else
        return "<Error token: " + T::message.as_string() + " at index " +
               std::to_string(T::position) + ">";
    };
    return (... + to_string(static_cast<Tokens *>(nullptr)));
  }
  static constexpr auto is_empty = (sizeof...(Tokens) == 0);
  static constexpr auto tokens_cnt = sizeof...(Tokens);
  template <std::size_t N>
    requires(N < tokens_cnt)
  struct nth {
    using type = std::decay_t<decltype(std::get<N>(std::tuple<Tokens...>{}))>;
  };
};

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
    static consteval auto get_result() noexcept {
      if constexpr (is_error_token<new_token>)
        return new_token{};
      else
        return typename parser<next_pos, CurrentTokens..., new_token>::result{};
    }
    using result = decltype(get_result());
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
  static consteval auto move() noexcept {
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
  template <CToken Token, std::size_t EndPos = static_cast<std::size_t>(-1)>
    requires(
        !(!is_error_token<Token> && EndPos == static_cast<std::size_t>(-1)))
  struct result_t {
    using token = Token;
    static constexpr auto end_pos = EndPos;
  };

 private:
  static consteval auto match_true() noexcept {
    if constexpr (Pos + 3 < Src.size() && Src[Pos + 1] == 'r' &&
                  Src[Pos + 2] == 'u' && Src[Pos + 3] == 'e')
      return result_t<True, Pos + 4>{};
    else
      return result_t<ErrorToken<"expects 'true'", Pos>>{};
  }
  static consteval auto match_false() noexcept {
    if constexpr (Pos + 4 < Src.size() && Src[Pos + 1] == 'a' &&
                  Src[Pos + 2] == 'l' && Src[Pos + 3] == 's' &&
                  Src[Pos + 4] == 'e')
      return result_t<False, Pos + 5>{};
    else
      return result_t<ErrorToken<"expects 'false'", Pos>>{};
  }
  static consteval auto match_null() noexcept {
    if constexpr (Pos + 3 < Src.size() && Src[Pos + 1] == 'u' &&
                  Src[Pos + 2] == 'l' && Src[Pos + 3] == 'l')
      return result_t<Null, Pos + 4>{};
    else
      return result_t<ErrorToken<"expects 'null'", Pos>>{};
  }

  struct string_matcher {
   private:
    static consteval auto first_scan() noexcept {
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
    static consteval auto get_contents() noexcept {
      char contents[end_quote_pos - Pos - escape_cnt];
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
      contents[fill] = '\0';
      return fixed_string(contents);
    }
    static constexpr auto first_scan_result = first_scan();
    static constexpr auto end_quote_pos = first_scan_result.first;
    static constexpr auto escape_cnt = first_scan_result.second;
    static consteval auto get_result() noexcept {
      if constexpr (end_quote_pos == Pos)
        return result_t<ErrorToken<"invalid string", Pos>>{};
      else if constexpr (escape_cnt == -1)
        return result_t<ErrorToken<"unsupported escape", end_quote_pos>>{};
      else
        return result_t<String<get_contents()>, end_quote_pos + 1>{};
    }

   public:
    using result = decltype(get_result());
  };

  struct integer_matcher {
   private:
    template <std::size_t Cur>
    struct next_nondigit_pos {
      static consteval auto get_result() noexcept {
        if constexpr (Cur < Src.size() && is_digit(Src[Cur]))
          return next_nondigit_pos<Cur + 1>::result;
        else
          return Cur;
      }
      static constexpr auto result = get_result();
    };
    template <std::size_t start, std::size_t end>
    struct calc_value {
      static constexpr std::size_t
          result = end - start > 10 ? 0
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
    static constexpr std::size_t value = calc_value<start_pos, end_pos>::result;
    static constexpr auto overflow = value >
                                     (2147483647ul + static_cast<int>(neg));

   public:
    using result = typename meta::switch_<
        0,
        meta::case_if<[](...) { return digits_length == 0; },
                      result_t<ErrorToken<"expects integer", start_pos>>>,
        meta::case_if<[](...) { return (digits_length > 10); },
                      result_t<ErrorToken<"integer too long", start_pos>>>,
        meta::case_if<
            [](...) { return too_many_leading_zeros; },
            result_t<ErrorToken<"too many leading zeros", start_pos>>>,
        meta::case_if<[](...) { return overflow; },
                      result_t<ErrorToken<"integer value exceeding the range "
                                          "of 32-bit signed integers",
                                          start_pos>>>,
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
      meta::default_<result_t<ErrorToken<"Unrecognized token", Pos>>>>::type;
};

/*
json    -> {value}
value   -> {object}
         | {array}
         | String
         | Integer
         | True
         | False
         | Null
object  -> LBrace RBrace
         | LBrace {members} RBrace
members -> {member}
         | {member} Comma {members}
member  -> {string} Colon {value}
array   -> LBracket RBracket
         | LBracket {values} RBracket
values  -> {value}
         | {value} Comma {values}
 */

template <typename... Members>
struct Object {};

template <typename... Values>
struct Array {};

template <fixed_string Key, typename Value>
struct Member {};

template <fixed_string Msg>
struct ParseError {
  static constexpr fixed_string message = Msg;
};

template <typename T>
inline constexpr auto is_parse_error = false;
template <fixed_string Msg>
inline constexpr auto is_parse_error<ParseError<Msg>> = true;

template <typename Tokens>
  requires(meta::specialization_of<Tokens, TokenSequence>)
struct Parser {
  template <std::size_t N>
  using nth_token = typename Tokens::template nth<N>::type;
};

} // namespace gkxx::constjson

#endif // GKXX_CONSTJSON_HPP