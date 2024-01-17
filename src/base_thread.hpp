#ifndef SVS_BASE_THREAD_HPP
#define SVS_BASE_THREAD_HPP

#include <cmath>
#include <mutex>
#include <queue>
#include <chrono>
#include <thread>
#include "structures.hpp"

namespace SVS{
class VirtualThread {
  public:
  void start() {
    std::thread t([this]() -> void {
      run();
    });
    t.detach();
  }
  protected:
  virtual void run() = 0;
};

class Timer {
  public:
  void start() {
    start_time_ = std::chrono::steady_clock::now();
    running_ = true;
  }
  void stop() {
    end_time_ = std::chrono::steady_clock::now();
    running_ = false;
  }
  double elapsed() {
    std::chrono::time_point<std::chrono::steady_clock> end_time;
    if (running_) {
      end_time = std::chrono::steady_clock::now();
    } else {
      end_time = end_time_;
    }
    return std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time_).count();
  }
  double msecsSinceStartOfLinux() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
  }
  private:
  std::chrono::time_point<std::chrono::steady_clock> start_time_;
  std::chrono::time_point<std::chrono::steady_clock> end_time_;
  bool running_ = false;
};

class BaseThread {
  public:
  void stop() {
    std::unique_lock<std::mutex> lock(stop_mutex_);
    stopped_ = true;
  }
  void update_fps(double dt) {
    if (dt > 0.0) {
      fps_.push(1000.0 / dt);
    }
    if (fps_.size() > FPS_STAT_QUEUE_LENGTH_) {
      fps_.pop();
    }
    if (fps_.size() == FPS_STAT_QUEUE_LENGTH_) {
      while(!fps_.empty()) {
        std::unique_lock<std::mutex> lock(processing_mutex_);
        fps_sum_ += fps_.front();
        fps_.pop();
      }
      stat_data_.average_fps_ = round(fps_sum_ / FPS_STAT_QUEUE_LENGTH_ * pow(10, 2)) / pow(10, 2);
      fps_sum_ = 0.0;
    }
  }
  protected:
  const int FPS_STAT_QUEUE_LENGTH_ = 32;
  bool stopped_ = false;
  std::mutex stop_mutex_;
  Timer clock_ = Timer();
  std::queue<double> fps_;
  double processing_time_ = 0.0;
  std::mutex processing_mutex_;
  double fps_sum_ = 0.0;
  ThreadStatisticsData stat_data_ = ThreadStatisticsData();
};

} //namespace SVS

#endif //SVS_BASE_THREAD_HPP