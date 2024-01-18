#ifndef SVS_PROCESS_THREAD_HPP
#define SVS_PROCESS_THREAD_HPP

#include <mutex>
#include <string>
#include <memory>
#include <iostream>
#include <stdexcept>
#include <opencv2/opencv.hpp>
#include "fisheye_camera.hpp"
#include "base_thread.hpp"
#include "imagebuffer.hpp"
#include "capture_thread.hpp"

namespace SVS{
class CameraProcessingThread : public BaseThread {
  public:
  CameraProcessingThread(std::shared_ptr<MultiBufferManager<Buffer<ImageFrame>, CaptureThread>> _sptr_capture_buffer_manager, std::shared_ptr<MultiBufferManager<Buffer<ImageFrame>, CameraProcessingThread>> _sptr_proc_buffer_manager, int _device_id, FisheyeCameraModel _camera_model, bool _drop_if_full=true)
    : sptr_capture_buffer_manager_(_sptr_capture_buffer_manager), sptr_proc_buffer_manager_(_sptr_proc_buffer_manager), device_id_(_device_id), camera_model_(_camera_model), drop_if_full_(_drop_if_full) {}
  void run() {
    if (sptr_proc_buffer_manager_ == std::shared_ptr<MultiBufferManager<Buffer<ImageFrame>, CameraProcessingThread>>(nullptr)){
      throw std::runtime_error("This thread has not been binded to any buffer manager yet");
    }
    while (true) {
      {
        std::unique_lock<std::mutex> lock{stop_mutex_};
        if (stopped_) {
          stopped_ = false;
          break;
        } 
      }
      processing_time_ = clock_.elapsed();
      clock_.start();
      {
        std::unique_lock<std::mutex> lock{processing_mutex_};
        ImageFrame raw_frame = sptr_capture_buffer_manager_->get_device(device_id_).get();
        cv::Mat und_frame  = camera_model_.undistort(raw_frame.image);
        cv::Mat pro_frame  = camera_model_.project(und_frame);
        cv::Mat flip_frame = camera_model_.flip(pro_frame);
      }
      sptr_proc_buffer_manager_.sync(device_id_);
      sptr_proc_buffer_manager_.set_frame_for_device(device_id_, flip_frame);
      update_fps(processing_time_);
      stat_data_.frames_processed_count_ += 1;
      //update_statistics_gui.emit(self.stat_data)
    }
  }
  protected:
  std::shared_ptr<MultiBufferManager<Buffer<ImageFrame>, CaptureThread>> sptr_capture_buffer_manager_       = std::shared_ptr<MultiBufferManager<Buffer<ImageFrame>, CaptureThread>>(nullptr);
  std::shared_ptr<MultiBufferManager<Buffer<ImageFrame>, CameraProcessingThread>> sptr_proc_buffer_manager_ = std::shared_ptr<MultiBufferManager<Buffer<ImageFrame>, CameraProcessingThread>>(nullptr);
  int device_id_;
  FisheyeCameraModel camera_model_;
  bool drop_if_full_;
};
}//namespace SVS
#endif