#ifndef SVS_IMAGEBUFFER_HPP
#define SVS_IMAGEBUFFER_HPP

#include <cassert>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <limits>
#include <mutex>
#include <queue>
#include <unordered_set>
#include <string>
#include <memory>
#include <opencv2/opencv.hpp>

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

template<typename StoredDataType>
class Buffer {
  public:
  Buffer(int _buffer_size = 5) : buffer_size_(_buffer_size) {
    free_slots_ = counting_semaphore<>{buffer_size_};
    used_slots_ = counting_semaphore<>{0};
    clear_buffer_add_ = counting_semaphore<>{1};
    clear_buffer_get_ = counting_semaphore<>{1};
  }
  void add(std::shared_ptr<StoredDataType> _sptr_data, bool _drop_if_full = false) {
    clear_buffer_add_.acquire();
    if (_drop_if_full) {
      if (free_slots_.try_acquire()) {
        {
          std::unique_lock<decltype(queue_mutex_)> lock{queue_mutex_};
          queue_.push(_sptr_data);
        }
        used_slots_.release();
      } else {
        free_slots_.acquire();
        {
          std::unique_lock<decltype(queue_mutex_)> lock{queue_mutex_};
          queue_.push(_sptr_data);
        }
        used_slots_.release();
      }
    }
    clear_buffer_add_.release();
  }
  std::shared_ptr<StoredDataType> get() {
    clear_buffer_get_.acquire();
    used_slots_.acquire();
    std::shared_ptr<StoredDataType> sptr_data;
    {
      std::unique_lock<decltype(queue_mutex_)> lock{queue_mutex_};
      sptr_data = queue_.front();
      queue_.pop();
    }
    free_slots_.release();
    clear_buffer_get_.release();
    return sptr_data;
  }
  bool clear() {
    if ((queue_.size()) > 0) {
      if (clear_buffer_add_.try_acquire()) {
        if (clear_buffer_get_.try_acquire()) {
          free_slots_.release(queue_.size());
          free_slots_.acquire(buffer_size_);
          used_slots_.acquire(queue_.size());
          for (int i=0; i<queue_.size(); i++) {
            queue_.pop();
          }
          free_slots_.release(buffer_size_);
          clear_buffer_get_.release();
        } else {
          return false;
        }
        clear_buffer_add_.release();
        return true;
      } else {
        return false;
      }
    } else {
      return false;
    }
  }
  int size() {
    return queue_.size();
  }
  int maxsize() {
    return buffer_size_;
  }
  bool isfull() {
    return queue_.size() == buffer_size_;
  }
  bool isempty() {
    return queue_.empty();
  }
  protected:
  int buffer_size_;
  counting_semaphore<> free_slots_, used_slots_, clear_buffer_add_, clear_buffer_get_;
  std::mutex queue_mutex_;
  std::queue<std::shared_ptr<StoredDataType>> queue_;
};

template<typename BufferType, typename ThreadType>
class MultiBufferManager {
  public:
  MultiBufferManager(bool _do_sync = true) : do_sync_(_do_sync) {}
  void create_buffer_for_device(int _device_id, int _buffer_size, bool _sync=true) {
    if (_sync) {
      std::unique_lock<decltype(mutex_)> lock{mutex_};
      sync_devices_.insert(_device_id);
    }
    buffer_maps_[_device_id] = BufferType(_buffer_size);
  }
  void bind_thread(std::shared_ptr<ThreadType> _sptr_thread, int _buffer_size, bool _sync=true) {
    create_buffer_for_device(_sptr_thread->device_id, _buffer_size, _sync);
    _sptr_thread.buffer_manager = this;
  }
  std::shared_ptr<BufferType> get_device(int _device_id) {
    return buffer_maps_[_device_id];
  }
  void remove_device(int _device_id) {
    buffer_maps_.erase(_device_id);
    {
      std::unique_lock<decltype(mutex_)> lock{mutex_};
      if (sync_devices_.find(_device_id) != sync_devices_.end()) {
        sync_devices_.erase(_device_id);
        cv_.notify_all();
      }
    }
  }
  void sync(int _device_id) {
    std::unique_lock<decltype(mutex_)> lock{mutex_};
    if (sync_devices_.find(_device_id) != sync_devices_.end()) {
      arrived_ += 1;
      if (do_sync_ && (arrived_ == sync_devices_.size())) {
        cv_.notify_all();
      } else {
        cv_.wait(lock);
      }
      arrived_ -= 1;
    }
  }
  void wake_all() {
    std::unique_lock<decltype(mutex_)> lock{mutex_};
    cv_.notify_all();
  }
  void set_sync(bool _enable) {
    do_sync_ = _enable;
  }
  bool sync_enabled() {
    return do_sync_;
  }
  bool sync_enabled_for_device(int _device_id) {
    return (sync_devices_.find(_device_id) != sync_devices_.end());
  }
  bool contains(int _device_id) {
    return buffer_maps_.find(_device_id) != buffer_maps_.end();
  }
  std::string str() {
    std::string keys, devices;
    for (const std::pair<int, BufferType> &item : buffer_maps_) {
      keys += (", " + std::to_string(item.first));
    }
    for (const int &item : sync_devices_) {
      devices += (", " + std::to_string(item));
    }
    return std::string(typeid(MultiBufferManager<BufferType, ThreadType>).name()) + ":\n" + \
           "sync: " + std::to_string(do_sync_) + "\n" + \
           "device: " + keys + "\n" + \
           "sync enabled devices: " + devices + "\n";
  }
  protected:
  std::unordered_set<int> sync_devices_;
  bool do_sync_;
  std::condition_variable cv_;
  std::mutex mutex_;
  std::map<int, std::shared_ptr<BufferType>> buffer_maps_;
  int arrived_ = 0;
};
} //namespace SVS

#endif //SVS_IMAGEBUFFER_HPP