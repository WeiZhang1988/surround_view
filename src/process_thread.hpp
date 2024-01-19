#ifndef SVS_PROCESS_THREAD_HPP
#define SVS_PROCESS_THREAD_HPP

#include <mutex>
#include <string>
#include <memory>
#include <iostream>
#include <stdexcept>
#include <opencv2/opencv.hpp>
#include "camera_model.hpp"
#include "base_thread.hpp"
#include "imagebuffer.hpp"
#include "capture_thread.hpp"

namespace SVS{
class CameraProcessingThread : public BaseThread {
  public:
  CameraProcessingThread(std::shared_ptr<MultiBufferManager<Buffer<ImageFrame>, CaptureThread>> _sptr_capture_buffer_manager, std::shared_ptr<ProjectedImageBufferManager<Buffer<std::shared_ptr<std::map<int, cv::Mat>>>, CameraProcessingThread>> _sptr_proc_buffer_manager, int _device_id, CameraModel _camera_model, bool _drop_if_full=true)
    : sptr_capture_buffer_manager_(_sptr_capture_buffer_manager), sptr_proc_buffer_manager_(_sptr_proc_buffer_manager), device_id_(_device_id), camera_model_(_camera_model), drop_if_full_(_drop_if_full) {}
  void run() {
    if (sptr_proc_buffer_manager_ == std::shared_ptr<ProjectedImageBufferManager<Buffer<std::shared_ptr<std::map<int, cv::Mat>>>, CameraProcessingThread>>(nullptr)){
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
      cv::Mat und_image, pro_image, flip_image;
      {
        std::unique_lock<std::mutex> lock{processing_mutex_};
        std::shared_ptr<ImageFrame> sptr_raw_frame = sptr_capture_buffer_manager_->get_device(device_id_)->get();
        und_image  = camera_model_.undistort(sptr_raw_frame->image_);
        pro_image  = camera_model_.project(und_image);
        flip_image = camera_model_.flip(pro_image);
      }
      sptr_proc_buffer_manager_->sync(device_id_);
      sptr_proc_buffer_manager_->set_image_for_device(device_id_, std::make_shared<cv::Mat>(flip_image));
      update_fps(processing_time_);
      stat_data_.frames_processed_count_ += 1;
      //update_statistics_gui.emit(self.stat_data)
    }
  }
  protected:
  std::shared_ptr<MultiBufferManager<Buffer<ImageFrame>, CaptureThread>> sptr_capture_buffer_manager_       = std::shared_ptr<MultiBufferManager<Buffer<ImageFrame>, CaptureThread>>(nullptr);
  std::shared_ptr<ProjectedImageBufferManager<Buffer<std::shared_ptr<std::map<int, cv::Mat>>>, CameraProcessingThread>> sptr_proc_buffer_manager_ = std::shared_ptr<ProjectedImageBufferManager<Buffer<std::shared_ptr<std::map<int, cv::Mat>>>, CameraProcessingThread>>(nullptr);
  int device_id_;
  CameraModel camera_model_;
  bool drop_if_full_;
};
}//namespace SVS
#endif