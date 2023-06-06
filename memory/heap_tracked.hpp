#ifndef GKXX_WHEEL_HEAP_TRACKED_HPP
#define GKXX_WHEEL_HEAP_TRACKED_HPP

#include <cstddef>
#include <stdexcept>
#include <unordered_set>

namespace gkxx {

class HeapTracked {
  using RawAddr = const void *;

 public:
  class InvalidDelete : public std::invalid_argument {
   public:
    InvalidDelete()
        : std::invalid_argument("deleting an object that is not heap-based.") {}
  };
  virtual ~HeapTracked() = 0;

  static void *operator new(std::size_t size);
  static void operator delete(void *ptr);
  bool is_on_heap() const;

 private:
  static std::unordered_set<RawAddr> addresses;
};

std::unordered_set<HeapTracked::RawAddr> HeapTracked::addresses;

HeapTracked::~HeapTracked() {}

void *HeapTracked::operator new(std::size_t size) {
  void *memPtr = ::operator new(size);
  addresses.insert(memPtr);
  return memPtr;
}

void HeapTracked::operator delete(void *ptr) {
  auto found = addresses.find(ptr);
  if (found != addresses.end()) {
    addresses.erase(found);
    ::operator delete(ptr);
  } else
    throw InvalidDelete{};
}

bool HeapTracked::is_on_heap() const {
  const void *ptr = dynamic_cast<const void *>(this);
  return addresses.find(ptr) != addresses.end();
}

} // namespace gkxx

#endif // GKXX_WHEEL_HEAP_TRACKED_HPP