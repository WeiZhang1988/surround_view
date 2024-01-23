#ifndef SVS_SEMAPHORE_HPP
#define SVS_SEMAPHORE_HPP

#include <cassert>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <limits>
#include <mutex>

namespace SVS{
template <std::ptrdiff_t least_max_value = std::numeric_limits<std::ptrdiff_t>::max()>
class counting_semaphore {
  public:
  static constexpr std::ptrdiff_t max() noexcept {
    static_assert(least_max_value >= 0, "least_max_value shall be non-negative");
    return least_max_value;
  }
  explicit counting_semaphore(std::ptrdiff_t _desired) : counter_(_desired) {
    assert(_desired >= 0 && _desired <= max());
  }
  ~counting_semaphore() = default;
  counting_semaphore(const counting_semaphore&) = delete;
  counting_semaphore &operator = (const counting_semaphore&) = delete;
  void acquire(int n=1) {
    std::unique_lock<decltype(mutex_)> lock{mutex_};
    cv_.wait(lock, [&]() {return counter_ > 0;});
    counter_ -= n;
  }
  bool try_acquire(int n=1) noexcept {
    std::unique_lock<decltype(mutex_)> lock{mutex_};
    if (counter_ <= 0) {
      return false;
    } else {
      counter_ -= n;
      return true;
    }
  }
  template<typename Rep, typename Period>
  bool try_acquire_for(const std::chrono::duration<Rep, Period> &_rel_time, int n=1) {
    return try_acquire_until(std::chrono::steady_clock::now() + _rel_time, n);
  }
  template <typename Clock, typename Duration>
  bool try_acquire_until(const std::chrono::time_point<Clock, Duration> &_timeout_time, int n=1) {
    std::unique_lock<decltype(mutex_)> lock{mutex_};
    if (!cv_.wait_until(lock, _timeout_time, [&]() {return counter_ > 0;})) {
      return false;
    } else {
      counter_ -= n;
      return true;
    }
  }
  void release(std::ptrdiff_t _update = 1) {
    {
      std::lock_guard<decltype(mutex_)> lock{mutex_};
      assert(_update >= 0 && _update <= max() - counter_);
      counter_ += _update;
      if (counter_ <= 0) {
        return;
      }
    }
    cv_.notify_all();
  }
  private:
  std::ptrdiff_t counter_{0};
  std::condition_variable cv_;
  std::mutex mutex_;
};
using binary_semaphore = counting_semaphore<1>;

}//namespace SVS

#endif 