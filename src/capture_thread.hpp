#ifndef SVS_CAPTURE_THREAD_HPP
#define SVS_CAPTURE_THREAD_HPP

#include <mutex>
#include <string>
#include <memory>
#include <iostream>
#include <stdexcept>
#include <opencv2/opencv.hpp>
#include "structures.hpp"
#include "base_thread.hpp"
#include "imagebuffer.hpp"

namespace SVS{
enum class FlipMethod {
  Identity,
  Rotation180,
};
std::string flipMethodToString(FlipMethod _flipmethod) {
  switch (_flipmethod) {
    case FlipMethod::Identity:
      return "Identity";
    case FlipMethod::Rotation180:
      return "Rotation180";
    default :
      return "unknown";
  }
}

class CaptureThread : public BaseThread {
  public:
  CaptureThread(std::shared_ptr<MultiBufferManager<Buffer<ImageFrame>, CaptureThread>> _sptr_buffer_manager, int _device_id, FlipMethod _flip_method, bool _drop_if_full, int _api_preference, cv::Size _resolution, bool _use_gst) 
    : sptr_buffer_manager_(_sptr_buffer_manager), device_id_(_device_id), flip_method_(_flip_method), drop_if_full_(_drop_if_full), api_preference_(_api_preference), resolution_(_resolution), use_gst_(_use_gst) {}
  bool connect_camera() {
    if (use_gst_) {
      cv::String options = gstreamer_pipeline(device_id_, 960, 640, 60, flip_method_);
      cap_.open(options, api_preference_);
    } else {
      cap_.open(device_id_);
    }
    if (cap_.isOpened()) {
      if (resolution_.width<=0 || resolution_.height<=0) {
        resolution_.width = int(cap_.get(cv::CAP_PROP_FRAME_WIDTH));
        resolution_.height = int(cap_.get(cv::CAP_PROP_FRAME_HEIGHT));
        cap_.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'));
      } else {
        cap_.set(cv::CAP_PROP_FRAME_WIDTH, resolution_.width);
        cap_.set(cv::CAP_PROP_FRAME_HEIGHT, resolution_.height);
        cap_.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'));
      }
    } else {
      std::cerr<<"Cannot open camera "<<std::to_string(device_id_)<<std::endl;
      return false;
    }
  } 
  bool disconnect_camera() {
    if (cap_.isOpened()) {
      cap_.release();
      return true;
    } else {
      return false;
    }
  }
  bool is_camera_connected() {
    return cap_.isOpened();
  }
  protected:
  void run() {
    if (sptr_buffer_manager_ == std::shared_ptr<MultiBufferManager<Buffer<ImageFrame>, CaptureThread>>(nullptr)){
      throw std::runtime_error("This thread has not been binded to any buffer manager yet");
    }
    while (true) {
      {
        std::unique_lock<std::mutex> lock(stop_mutex_);
        if (stopped_) {
          stopped_ = false;
          break;
        }
      }
      processing_time_ = clock_.elapsed();
      clock_.start();
      sptr_buffer_manager_->sync(device_id_);
      if (cap_.grab()) {
        cv::Mat frame;
        bool ret = cap_.retrieve(frame);
        std::shared_ptr<ImageFrame> sptr_img_frame = std::make_shared<ImageFrame>(clock_.msecsSinceStartOfLinux(), frame);
        sptr_buffer_manager_->get_device(device_id_)->add(sptr_img_frame, drop_if_full_);
        update_fps(processing_time_);
        stat_data_.frames_processed_count_ += 1;
        //update_statistics_gui_.emit(stat_data_);
      }
    }
  }
  cv::String gstreamer_pipeline(int _cam_id = 0, int _capture_width = 960, int _capture_height = 640, int _framerate = 60, FlipMethod _flip_method = FlipMethod::Rotation180) {
    return "nvarguscamerasrc sensor-id=" + std::to_string(_cam_id) + " ! " + \
           "video/x-raw(memory:NVMM), " + \
           "width=(int)" + std::to_string(_capture_width) + ", height=(int)" + std::to_string(_capture_height) + ", " + \
           "format=(string)NV12, framerate=(fraction)" + std::to_string(_framerate) + "/1 ! " + \
           "nvvidconv flip-method=" + flipMethodToString(_flip_method) + " ! " + \
           "video/x-raw, format=(string)BGRx ! " + \
           "videoconvert ! " + \
           "video/x-raw, format=(string)BGR ! appsink";
  }
  int device_id_;
  FlipMethod flip_method_;
  bool drop_if_full_;
  int api_preference_;
  cv::Size resolution_;
  bool use_gst_;
  cv::VideoCapture cap_ = cv::VideoCapture();
  std::shared_ptr<MultiBufferManager<Buffer<ImageFrame>, CaptureThread>> sptr_buffer_manager_ = std::shared_ptr<MultiBufferManager<Buffer<ImageFrame>, CaptureThread>>(nullptr);
};

} //namespace SVS

#endif //SVS_CAPTURE_THREAD_HPP