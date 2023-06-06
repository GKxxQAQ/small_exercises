#include <cstdlib>
#include <functional>
#include <memory>
#include <unordered_set>

#include "mallocator.hpp"

namespace detail {

struct Recorder {
  std::unordered_set<void *, std::hash<void *>, std::equal_to<void *>,
                     gkxx::Mallocator<void *>>
      addrs;

  ~Recorder() {
    for (auto p : addrs)
      std::free(p);
  }
};

static Recorder rec_new, rec_array_new;

#if CHECK_DEALLOCATION
static std::string to_string(void *ptr) {
  static constexpr auto buffer_size = sizeof(void *) / 4 + 4;
  static char tmp[buffer_size] = {};
  std::sprintf(tmp, "%p", ptr);
  return {tmp};
}
#endif

void add_new(void *ptr) {
  if (ptr)
    rec_new.addrs.insert(ptr);
}

void add_array_new(void *ptr) {
  if (ptr)
    rec_array_new.addrs.insert(ptr);
}

void remove_new(void *ptr) {
  if (!ptr)
    return;
#ifdef CHECK_DEALLOCATION
  if (rec_new.addrs.erase(ptr) == 0)
    throw std::runtime_error{
        "`operator delete` called on address " + to_string(ptr) +
        " that is not previously obtained from operator new."};
#else
  rec_new.addrs.erase(ptr);
#endif
}

void remove_array_new(void *ptr) {
  if (!ptr)
    return;
#ifdef CHECK_DEALLOCATION
  if (rec_array_new.addrs.erase(ptr) == 0)
    throw std::runtime_error{
        "`operator delete[]` called on address " + to_string(ptr) +
        " that is not previously obtained from operator new[]."};
#else
  rec_array_new.addrs.erase(ptr);
#endif
}

} // namespace detail