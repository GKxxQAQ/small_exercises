#include <exception>
#include <memory>
#include <type_traits>

struct bad_function_call : public std::exception {
  const char *what() const noexcept override {
    return "bad function call";
  }
};

template <typename>
class function;

template <typename R, typename... Args>
class function<R(Args...)> {
 public:
  template <typename Fn>
  function(Fn &&fn)
      : pimpl(
            std::make_unique<wrapper<std::decay_t<Fn>>>(std::forward<Fn>(fn))) {
  }

  function(const function &other) : pimpl(other.pimpl->clone()) {}
  function(function &&) noexcept = default;
  void swap(function &other) noexcept {
    pimpl.swap(other);
  }
  function &operator=(function other) noexcept {
    swap(other);
    return *this;
  }
  ~function() = default;

  R operator()(Args... args) const {
    if (!pimpl)
      throw bad_function_call{};
    return pimpl->invoke(std::forward<Args>(args)...);
  }

 private:
  struct wrapper_base {
    virtual ~wrapper_base() = default;
    virtual std::unique_ptr<wrapper_base> clone() const = 0;
    virtual R invoke(Args... args) const = 0;
  };
  template <typename F>
  struct wrapper final : public wrapper_base {
    F func;
    template <typename Fn>
    wrapper(Fn &&fn) : func(std::forward<Fn>(fn)) {}
    std::unique_ptr<wrapper_base> clone() const final {
      return std::make_unique<wrapper<F>>(func);
    }
    R invoke(Args... args) const final {
      return func(std::forward<Args>(args)...);
    }
    ~wrapper() = default;
  };
  std::unique_ptr<wrapper_base> pimpl;
};

#include <functional>
#include <iostream>

struct Foo {
  Foo(int num) : num_(num) {}
  void print_add(int i) const {
    std::cout << num_ + i << '\n';
  }
  int num_;
};

void print_num(int i) {
  std::cout << i << '\n';
}

struct PrintNum {
  void operator()(int i) const {
    std::cout << i << '\n';
  }
};

int main() {
  // store a free function
  function<void(int)> f_display = print_num;
  f_display(-9);

  // store a lambda
  function<void()> f_display_42 = []() { print_num(42); };
  f_display_42();

  // store the result of a call to std::bind
  function<void()> f_display_31337 = std::bind(print_num, 31337);
  f_display_31337();

  // store a call to a function object
  function<void(int)> f_display_obj = PrintNum();
  f_display_obj(18);

  auto factorial = [](int n) {
    // store a lambda object to emulate "recursive lambda"; aware of extra
    // overhead
    function<int(int)> fac = [&](int n) {
      return (n < 2) ? 1 : n * fac(n - 1);
    };
    // note that "auto fac = [&](int n) {...};" does not work in recursive calls
    return fac(n);
  };
  for (int i{5}; i != 8; ++i)
    std::cout << i << "! = " << factorial(i) << ";  ";
  std::cout << '\n';
}