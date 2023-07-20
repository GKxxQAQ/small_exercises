#include <coroutine>
#include <iostream>
#include <queue>
#include <string>
#include <utility>

struct Scheduler {
  std::queue<std::coroutine_handle<>> m_tasks{};
  bool schedule() {
    if (m_tasks.empty())
      return false;
    auto task = m_tasks.front();
    m_tasks.pop();
    if (!task.done())
      task.resume();
    return !m_tasks.empty();
  }
  auto suspend() {
    struct awaiter : std::suspend_always {
      Scheduler &scheduler;
      constexpr awaiter(Scheduler &s) noexcept : scheduler{s} {}
      void await_suspend(std::coroutine_handle<> handle) {
        scheduler.m_tasks.push(handle);
      }
    };
    return awaiter{*this};
  }
};

struct Task {
  struct promise_type {
    std::suspend_never initial_suspend() const noexcept {
      return {};
    }
    std::suspend_never final_suspend() const noexcept {
      return {};
    }
    void unhandled_exception() const noexcept {}
    Task get_return_object() const noexcept {
      return {};
    }
  };
  constexpr Task() noexcept = default;
};

struct Task_function {
  std::string name;

  constexpr Task_function(std::string s) : name{std::move(s)} {}
  
  Task operator()(Scheduler &scheduler) const noexcept {
    std::cout << "Hello, from task " << name << "\n";
    co_await scheduler.suspend();
    std::cout << name << " is back doing work\n";
    co_await scheduler.suspend();
    std::cout << name << " is back doing more work\n";
  }
};

int main() {
  Scheduler scheduler{};
  auto a = Task_function{"A"};
  auto b = Task_function{"B"};
  auto c = Task_function{"C"};
  a(scheduler);
  b(scheduler);
  c(scheduler);
  while (scheduler.schedule())
    ;
  return 0;
}