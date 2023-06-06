#ifndef GKXX_AUTO_GC_HPP
#define GKXX_AUTO_GC_HPP

#if __cplusplus < 201103L
#error At least C++11 is required.
#endif

#include <cstddef>
#include <cstdlib>
#include <functional>
#include <memory>
#include <new>
#include <unordered_set>

namespace detail {

inline void *aligned_allocate(std::size_t size, std::size_t alignment) {
  void *p;
#ifdef _WIN32
  p = _aligned_malloc(size, alignment);
#else
  if (::posix_memalign(&p, alignment, size) != 0)
    p = nullptr;
#endif
  return p;
}

template <typename AllocFunc, typename... Args>
void *new_loop(AllocFunc &&func, std::size_t size, Args &&...args) {
  if (size == 0)
    size = 1;
  void *ptr;
  while ((ptr = std::forward<AllocFunc>(func)(
              size, std::forward<Args>(args)...)) == nullptr) {
    auto nh = std::get_new_handler();
    if (!nh)
      throw std::bad_alloc{};
    nh();
  }
  return ptr;
}

void add_new(void *ptr);
void add_array_new(void *ptr);
void remove_new(void *ptr);
void remove_array_new(void *ptr);

} // namespace detail

#if __cplusplus >= 202002L
#define NEW_NODISCARD [[nodiscard]]
#else
#define NEW_NODISCARD
#endif

// operator new replacements

NEW_NODISCARD void *operator new(std::size_t count) {
  auto ptr = detail::new_loop(std::malloc, count);
  detail::add_new(ptr);
  return ptr;
}

NEW_NODISCARD void *operator new[](std::size_t count) {
  auto ptr = detail::new_loop(std::malloc, count);
  detail::add_array_new(ptr);
  return ptr;
}

NEW_NODISCARD void *operator new(std::size_t count,
                                 const std::nothrow_t &) noexcept {
  void *ptr{};
  try {
    ptr = ::operator new(count);
  } catch (...) {
  }
  return ptr;
}

NEW_NODISCARD void *operator new[](std::size_t count,
                                   const std::nothrow_t &) noexcept {
  void *ptr{};
  try {
    ptr = ::operator new[](count);
  } catch (...) {
  }
  return ptr;
}

#if __cpp_aligned_new

NEW_NODISCARD void *operator new(std::size_t count, std::align_val_t al) {
  auto alignment = static_cast<std::size_t>(al);
  if (alignment < sizeof(void *))
    alignment = sizeof(void *);
  auto ptr = detail::new_loop(detail::aligned_allocate, count, alignment);
  detail::add_new(ptr);
  return ptr;
}

NEW_NODISCARD void *operator new[](std::size_t count, std::align_val_t al) {
  auto alignment = static_cast<std::size_t>(al);
  if (alignment < sizeof(void *))
    alignment = sizeof(void *);
  auto ptr = detail::new_loop(detail::aligned_allocate, count, alignment);
  detail::add_array_new(ptr);
  return ptr;
}

NEW_NODISCARD void *operator new(std::size_t count, std::align_val_t al,
                                 const std::nothrow_t &) noexcept {
  void *ptr{};
  try {
    ptr = ::operator new(count, al);
  } catch (...) {
  }
  return ptr;
}

NEW_NODISCARD void *operator new[](std::size_t count, std::align_val_t al,
                                   const std::nothrow_t &) noexcept {
  void *ptr{};
  try {
    ptr = ::operator new[](count, al);
  } catch (...) {
  }
  return ptr;
}

#endif // __cpp_aligned_new

// operator delete replacements

void operator delete(void *ptr) noexcept {
  std::cout << ptr << std::endl;
  detail::remove_new(ptr);
  std::free(ptr);
}

void operator delete[](void *ptr) noexcept {
  detail::remove_array_new(ptr);
  std::free(ptr);
}

void operator delete(void *ptr, const std::nothrow_t &) noexcept {
  ::operator delete(ptr);
}

void operator delete[](void *ptr, const std::nothrow_t &) noexcept {
  ::operator delete[](ptr);
}

#if __cpp_sized_deallocation

void operator delete(void *ptr, std::size_t) noexcept {
  ::operator delete(ptr);
}

void operator delete[](void *ptr, std::size_t) noexcept {
  ::operator delete[](ptr);
}

#endif // __cpp_sized_deallocation

#if __cpp_aligned_new

void operator delete(void *ptr, std::align_val_t) noexcept {
  ::operator delete(ptr);
}

void operator delete[](void *ptr, std::align_val_t) noexcept {
  ::operator delete[](ptr);
}

void operator delete(void *ptr, std::align_val_t al,
                     const std::nothrow_t &) noexcept {
  ::operator delete(ptr, al);
}

void operator delete[](void *ptr, std::align_val_t al,
                       const std::nothrow_t &) noexcept {
  ::operator delete[](ptr, al);
}

#if __cpp_sized_deallocation

void operator delete(void *ptr, std::size_t, std::align_val_t al) noexcept {
  ::operator delete(ptr, al);
}

void operator delete[](void *ptr, std::size_t, std::align_val_t al) noexcept {
  ::operator delete[](ptr, al);
}

#endif // __cpp_sized_deallocation

#endif // __cpp_aligned_new

#endif // GKXX_AUTO_GC_HPP