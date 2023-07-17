#ifndef GKXX_CONSTJSON_HPP
#define GKXX_CONSTJSON_HPP

#include <concepts>
#include <string>
#include <tuple>
#include <utility>

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
  static constexpr auto to_string() {
    return std::to_string(value);
  }
  static constexpr auto to_pretty_string(std::size_t indent) {
    return std::string(indent, ' ') + "Int<" + std::to_string(value) + ">";
  }
};

template <fixed_string S>
struct String {
  static constexpr fixed_string value = S;
  static constexpr auto to_string() {
    return value.to_string();
  }
  static constexpr auto to_pretty_string(std::size_t indent) {
    return std::string(indent, ' ') + "String<" + value.to_string() + ">";
  }
};

struct True {
  static constexpr auto to_fixed_string() noexcept {
    return fixed_string("true");
  }
  static constexpr auto to_pretty_string(std::size_t indent) {
    return std::string(indent, ' ') + "True";
  }
};
struct False {
  static constexpr auto to_fixed_string() noexcept {
    return fixed_string("false");
  }
  static constexpr auto to_pretty_string(std::size_t indent) {
    return std::string(indent, ' ') + "False";
  }
};
struct Null {
  static constexpr auto to_fixed_string() noexcept {
    return fixed_string("null");
  }
  static constexpr auto to_pretty_string(std::size_t indent) {
    return std::string(indent, ' ') + "Null";
  }
};

struct LBrace {
  static constexpr auto to_fixed_string() noexcept {
    return fixed_string("{");
  }
};
struct RBrace {
  static constexpr auto to_fixed_string() noexcept {
    return fixed_string("}");
  }
};
struct LBracket {
  static constexpr auto to_fixed_string() noexcept {
    return fixed_string("[");
  }
};
struct RBracket {
  static constexpr auto to_fixed_string() noexcept {
    return fixed_string("]");
  }
};
struct Comma {
  static constexpr auto to_fixed_string() noexcept {
    return fixed_string(",");
  }
};
struct Colon {
  static constexpr auto to_fixed_string() noexcept {
    return fixed_string(":");
  }
};

template <fixed_string Msg, std::size_t Pos>
struct ErrorToken {
  static constexpr fixed_string message = Msg;
  static constexpr std::size_t position = Pos;
};

namespace detail {

  template <typename T>
  inline constexpr auto is_integer_token = false;
  template <int N>
  inline constexpr auto is_integer_token<Integer<N>> = true;

  template <typename T>
  inline constexpr auto is_string_token = false;
  template <fixed_string S>
  inline constexpr auto is_string_token<String<S>> = true;

  template <typename T>
  inline constexpr auto is_error_token = false;
  template <fixed_string Msg, std::size_t Pos>
  inline constexpr auto is_error_token<ErrorToken<Msg, Pos>> = true;

  template <typename T, typename... Types>
  concept same_as_any = (... || std::same_as<T, Types>);

} // namespace detail

template <typename T>
concept CToken = detail::same_as_any<T, True, False, Null, LBrace, RBrace,
                                     LBracket, RBracket, Comma, Colon> ||
                 detail::is_integer_token<T> || detail::is_string_token<T> ||
                 detail::is_error_token<T>;

template <CToken... Tokens>
struct TokenSequence {
  static constexpr std::string reconstruct_string() {
    auto to_string = []<typename T>(T *) -> std::string {
      if constexpr (detail::is_integer_token<T>)
        return std::to_string(T::value);
      else if constexpr (detail::is_string_token<T>)
        return "\"" + T::value.to_string() + "\"";
      else if constexpr (detail::same_as_any<T, True, False, Null, LBrace,
                                             RBrace, LBracket, RBracket, Comma,
                                             Colon>)
        return T::to_fixed_string().to_string();
      else
        return "<Error token: " + T::message.to_string() + " at index " +
               std::to_string(T::position) + ">";
    };
    return (... + to_string(static_cast<Tokens *>(nullptr)));
  }
  static constexpr auto empty = (sizeof...(Tokens) == 0);
  static constexpr auto size = sizeof...(Tokens);
  template <std::size_t N>
    requires(N < size)
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
  struct lexer {
    // Src[Pos] is non-whitespace
    using token_getter = typename token_getter<Pos>::result;
    using new_token = typename token_getter::token;
    static constexpr auto next_pos =
        next_nonwhitespace_pos<token_getter::end_pos>::result;
    static consteval auto get_result() noexcept {
      if constexpr (detail::is_error_token<new_token>)
        return new_token{};
      else
        return typename lexer<next_pos, CurrentTokens..., new_token>::result{};
    }
    using result = decltype(get_result());
  };
  template <CToken... Tokens>
  struct lexer<Src.size(), Tokens...> {
    using result = TokenSequence<Tokens...>;
  };

  using result = lexer<next_nonwhitespace_pos<0>::result>::result;
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
    requires(!(!detail::is_error_token<Token> &&
               EndPos == static_cast<std::size_t>(-1)))
  struct internal_result_t {
    using token = Token;
    static constexpr auto end_pos = EndPos;
  };

  template <fixed_string Msg, std::size_t ErrorPos>
  using error_result_t = internal_result_t<ErrorToken<Msg, ErrorPos>>;

 private:
  static consteval auto match_true() noexcept {
    if constexpr (Pos + 3 < Src.size() && Src[Pos + 1] == 'r' &&
                  Src[Pos + 2] == 'u' && Src[Pos + 3] == 'e')
      return internal_result_t<True, Pos + 4>{};
    else
      return error_result_t<"expects 'true'", Pos>{};
  }
  static consteval auto match_false() noexcept {
    if constexpr (Pos + 4 < Src.size() && Src[Pos + 1] == 'a' &&
                  Src[Pos + 2] == 'l' && Src[Pos + 3] == 's' &&
                  Src[Pos + 4] == 'e')
      return internal_result_t<False, Pos + 5>{};
    else
      return error_result_t<"expects 'false'", Pos>{};
  }
  static consteval auto match_null() noexcept {
    if constexpr (Pos + 3 < Src.size() && Src[Pos + 1] == 'u' &&
                  Src[Pos + 2] == 'l' && Src[Pos + 3] == 'l')
      return internal_result_t<Null, Pos + 4>{};
    else
      return error_result_t<"expects 'null'", Pos>{};
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
        return error_result_t<"invalid string", Pos>{};
      else if constexpr (escape_cnt == -1)
        return error_result_t<"unsupported escape", end_quote_pos>{};
      else
        return internal_result_t<String<get_contents()>, end_quote_pos + 1>{};
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
                      error_result_t<"expects integer", start_pos>>,
        meta::case_if<[](...) { return (digits_length > 10); },
                      error_result_t<"integer too long", start_pos>>,
        meta::case_if<[](...) { return too_many_leading_zeros; },
                      error_result_t<"too many leading zeros", start_pos>>,
        meta::case_if<[](...) { return overflow; },
                      error_result_t<"integer value exceeding the range "
                                     "of 32-bit signed integers",
                                     start_pos>>,
        meta::default_<internal_result_t<
            Integer<neg ? -static_cast<int>(value) : static_cast<int>(value)>,
            end_pos>>>::type;
  };

 public:
  using result = typename meta::switch_<
      Src[Pos], meta::case_<'{', internal_result_t<LBrace, Pos + 1>>,
      meta::case_<'}', internal_result_t<RBrace, Pos + 1>>,
      meta::case_<'[', internal_result_t<LBracket, Pos + 1>>,
      meta::case_<']', internal_result_t<RBracket, Pos + 1>>,
      meta::case_<',', internal_result_t<Comma, Pos + 1>>,
      meta::case_<':', internal_result_t<Colon, Pos + 1>>,
      meta::case_<'t', decltype(match_true())>,
      meta::case_<'f', decltype(match_false())>,
      meta::case_<'n', decltype(match_null())>,
      meta::case_<'"', typename string_matcher::result>,
      meta::case_if<[](char c) { return c == '-' || is_digit(c); },
                    typename integer_matcher::result>,
      meta::default_<error_result_t<"Unrecognized token", Pos>>>::type;
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
member  -> String Colon {value}
array   -> LBracket RBracket
         | LBracket {values} RBracket
values  -> {value}
         | {value} Comma {values}
 */

template <typename... Members>
struct Object {
  static constexpr auto to_pretty_string(std::size_t indent) {
    return std::string(indent, ' ') + "Object<\n" +
           (... + Members::to_pretty_string(indent + 2)) +
           std::string(indent, ' ') + ">\n";
  }
};

template <typename... Values>
struct Array {
  template <typename...>
  struct make_comma_sep_list;

  template <typename First, typename... Rest>
  struct make_comma_sep_list<First, Rest...> {
    constexpr auto operator()(std::size_t indent) {
      return First::to_pretty_string(indent) +
             (... + (", " + Rest::to_pretty_string(indent)));
    }
  };

  template <typename First>
  struct make_comma_sep_list<First> {
    constexpr auto operator()(std::size_t indent) {
      return First::to_pretty_string(indent);
    }
  };

  template <>
  struct make_comma_sep_list<> {
    constexpr auto operator()([[maybe_unused]] std::size_t indent) {
      return std::string{};
    }
  };

  static constexpr auto to_pretty_string(std::size_t indent) {
    return std::string(indent, ' ') + "Array<" +
           make_comma_sep_list<Values...>{}(indent) + ">\n";
  }
};

template <fixed_string Key, typename Value>
struct Member {
  static constexpr auto to_pretty_string(std::size_t indent) {
    return std::string(indent, ' ') + "Member<" + Key.to_string() + ", " +
           Value::to_pretty_string(indent) + ">\n";
  }
};

struct EndOfTokens {};

template <fixed_string Msg>
struct SyntaxError {
  static constexpr fixed_string message = Msg;
};

namespace detail {

  template <typename T>
  inline constexpr auto is_member = false;

  template <fixed_string Key, typename Value>
  inline constexpr auto is_member<Member<Key, Value>> = true;

  template <typename T>
  inline constexpr auto is_syntax_error = false;
  template <fixed_string Msg>
  inline constexpr auto is_syntax_error<SyntaxError<Msg>> = true;

  template <typename T>
  concept CTerminal = is_string_token<T> || is_integer_token<T> ||
                      same_as_any<T, True, False, Null>;

} // namespace detail

template <typename Tokens>
struct ParseTokens {
  static_assert(meta::specialization_of<Tokens, TokenSequence>);

  template <std::size_t N>
  using nth_token = decltype([]() {
    if constexpr (N < Tokens::size)
      return typename Tokens::template nth<N>::type{};
    else
      return EndOfTokens{};
  }());

  template <typename ParseResult,
            std::size_t NextPos = static_cast<std::size_t>(-1)>
    requires(!(!detail::is_syntax_error<ParseResult> &&
               NextPos == static_cast<std::size_t>(-1)))
  struct internal_result_t {
    using node = ParseResult;
    static constexpr auto next_pos = NextPos;
  };

  template <fixed_string Msg>
  using error_result_t = internal_result_t<SyntaxError<Msg>>;

  template <std::size_t Pos>
  struct value_parser;

  template <std::size_t Pos>
  struct member_parser;

  template <std::size_t Pos, template <std::size_t> typename element_parser,
            template <typename...> typename list_type>
  struct comma_list_parser;

  template <std::size_t Pos, typename Left, typename Right,
            template <std::size_t> typename list_parser>
  struct bracket_pair_list_parser;

  template <std::size_t Pos>
  struct members_parser : comma_list_parser<Pos, member_parser, Object> {};

  template <std::size_t Pos>
  struct values_parser : comma_list_parser<Pos, value_parser, Array> {};

  template <std::size_t Pos>
  struct array_parser
      : bracket_pair_list_parser<Pos, LBracket, RBracket, values_parser> {};

  template <std::size_t Pos>
  struct object_parser
      : bracket_pair_list_parser<Pos, LBrace, RBrace, members_parser> {};

  using result = typename value_parser<0>::result;
};

template <typename Tokens>
template <std::size_t Pos>
struct ParseTokens<Tokens>::value_parser {
  static consteval auto do_parse() noexcept {
    using lookahead = nth_token<Pos>;
    if constexpr (std::is_same_v<lookahead, EndOfTokens>)
      return internal_result_t<SyntaxError<"expects Value">>{};
    else if constexpr (std::is_same_v<lookahead, LBrace>)
      return typename object_parser<Pos>::result{};
    else if constexpr (std::is_same_v<lookahead, LBracket>)
      return typename array_parser<Pos>::result{};
    else {
      static_assert(detail::CTerminal<lookahead>);
      return internal_result_t<lookahead, Pos + 1>{};
    }
  }
  using result = decltype(do_parse());
};

template <typename Tokens>
template <std::size_t Pos, typename Left, typename Right,
          template <std::size_t> typename list_parser>
struct ParseTokens<Tokens>::bracket_pair_list_parser {
  static consteval auto do_parse() noexcept {
    using lookahead = nth_token<Pos>;
    if constexpr (!std::is_same_v<lookahead, Left>)
      return error_result_t<"expects '" + Left::to_fixed_string() + "'">{};
    else
      return match_after_left();
  }
  static consteval auto match_after_left() noexcept {
    using lookahead = nth_token<Pos + 1>;
    if constexpr (std::is_same_v<lookahead, Right>)
      return internal_result_t<Object<>, Pos + 2>{};
    else {
      using elements_result = typename list_parser<Pos + 1>::result;
      using elements_node = typename elements_result::node;
      if constexpr (detail::is_syntax_error<elements_node>)
        return elements_result{};
      else {
        constexpr auto next_pos = elements_result::next_pos;
        using next_token = nth_token<next_pos>;
        if constexpr (std::is_same_v<next_token, RBrace>)
          return internal_result_t<elements_node, next_pos + 1>{};
        else
          return error_result_t<"expects '" + Right::to_fixed_string() + "'">{};
      }
    }
  }
  using result = decltype(do_parse());
};

template <typename Tokens>
template <std::size_t Pos>
struct ParseTokens<Tokens>::member_parser {
  static consteval auto do_parse() noexcept {
    using lookahead = nth_token<Pos>;
    if constexpr (!detail::is_string_token<lookahead>)
      return error_result_t<"expects String">{};
    else
      return match_colon();
  }
  static consteval auto match_colon() noexcept {
    using lookahead = nth_token<Pos + 1>;
    if constexpr (!std::is_same_v<lookahead, Colon>)
      return error_result_t<"expects ':'">{};
    else
      return match_value();
  }
  static consteval auto match_value() noexcept {
    using value_result = typename value_parser<Pos + 2>::result;
    using value_node = typename value_result::node;
    if constexpr (detail::is_syntax_error<value_node>)
      return value_result{};
    else
      return internal_result_t<Member<nth_token<Pos>::value_result, value_node>,
                               value_result::next_pos>{};
  }
  using result = decltype(do_parse());
};

template <typename Tokens>
template <std::size_t Pos, template <std::size_t> typename element_parser,
          template <typename...> typename list_type>
struct ParseTokens<Tokens>::comma_list_parser {
  template <std::size_t CurPos, typename... CurElems>
  struct parse {
    static consteval auto get_result() noexcept {
      using new_elem_result = typename element_parser<CurPos>::result;
      using new_elem_node = typename new_elem_result::node;
      if constexpr (detail::is_syntax_error<new_elem_node>)
        return new_elem_result{};
      else {
        constexpr auto next_pos = new_elem_result::next_pos;
        using next_token = nth_token<next_pos>;
        if constexpr (std::is_same_v<next_token, Comma>)
          return typename parse<next_pos + 1, CurElems...,
                                new_elem_node>::result{};
        else
          return internal_result_t<list_type<CurElems..., new_elem_node>,
                                   next_pos>{};
      }
    }
    using result = decltype(get_result());
  };
};

template <fixed_string JsonCode>
struct parse {
  static consteval auto get_result() noexcept {
    using tokenize_result = Tokenizer<JsonCode>::result;
    if constexpr (detail::is_error_token<tokenize_result>)
      return tokenize_result{};
    else {
      using parse_result = typename ParseTokens<tokenize_result>::result;
      using root_node = typename parse_result::node;
      constexpr auto next_pos = parse_result::next_pos;
      if constexpr (detail::is_syntax_error<root_node>)
        return root_node{};
      else if constexpr (next_pos < tokenize_result::size)
        return SyntaxError<"expects end of string">{};
      else
        return root_node{};
    }
  }
  using result = decltype(get_result());
};

} // namespace gkxx::constjson

#endif // GKXX_CONSTJSON_HPP