#ifndef SVS_BASE_THREAD_HPP
#define SVS_BASE_THREAD_HPP

#include <opencv2/opencv.hpp>
#include <queue>
#include <chrono>
#include <thread>
#include "structures.hpp"

namespace SVS{
class BaseThread {
  public:
  BaseThread() {
    
  }
  void start() {
    run_flag_ = true;
    std::thread t([this]() -> void {
      while(run_flag_){
        run();
      }
    });
    t.detach();
  }
  void stop() {run_flag_ = false;}
  virtual void run() {}
  protected:
  bool run_flag_ = false;
};

} //namespace SVS

#endif //SVS_BASE_THREAD_HPP