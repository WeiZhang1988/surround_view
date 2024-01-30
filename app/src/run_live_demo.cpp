#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <sys/stat.h>
#include <opencv2/opencv.hpp>
#include "camera_model.hpp"
#include "capture_thread.hpp"
#include "process_thread.hpp"
#include "birdview.hpp"
#include "imagebuffer.hpp"
#include "utils.hpp"

using namespace SVS;

std::string yamls_dir = "./data/yamls";
std::vector<int> camera_ids{4,3,5,6};
std::vector<FlipMethod> flip_methods{FlipMethod::Identity,FlipMethod::Rotation180,FlipMethod::Identity,FlipMethod::Rotation180};
std::vector<std::string> names = static_settings.camera_names;
std::vector<std::string> camera_files;

int main(int argc, char *argv[]) {
  for (std::string name : names) {
    camera_files.push_back(yamls_dir+name+".yaml");
  }
  std::vector<std::shared_ptr<CameraModel>> camera_models;
  for (int i=0;i<names.size();i++) {
    camera_models.push_back(std::make_shared<CameraModel>(camera_files[i],names[i]));
  }
  std::vector<std::shared_ptr<CaptureThread>> capture_threads;
  for (int i=0;i<names.size();i++) {
    capture_threads.push_back(std::make_shared<CaptureThread>(camera_ids[i],flip_methods[i],true,cv::CAP_GSTREAMER,cv::Size(),false));
  }
  std::shared_ptr<MultiBufferManager<ImageFrame,CaptureThread>> sptr_capture_buffer_manager= std::make_shared<MultiBufferManager<ImageFrame,CaptureThread>>();
  for (std::shared_ptr<CaptureThread> sptr_capture_thread : capture_threads) {
    sptr_capture_buffer_manager->create_buffer_and_bind_thread(sptr_capture_thread, 8, true);
    if(sptr_capture_thread->connect_camera()) {
      sptr_capture_thread->start();
    }
  }
  std::shared_ptr<ProjectedImageBufferManager> sptr_proc_buffer_manager = std::make_shared<ProjectedImageBufferManager>();
  std::vector<std::shared_ptr<CameraProcessingThread>> process_threads;
  for (int i=0;i<names.size();i++) {
    process_threads.push_back(std::make_shared<CameraProcessingThread>(camera_ids[i], camera_models[i], true));
  }
  for (std::shared_ptr<CameraProcessingThread> sptr_process_thread : process_threads) {
    sptr_proc_buffer_manager->create_and_bind_thread<CameraProcessingThread>(sptr_process_thread);
    sptr_process_thread->start();
  }
  std::shared_ptr<BirdView> sptr_birdview_thread= std::make_shared<BirdView>();
  sptr_proc_buffer_manager->bind_thread<BirdView>(sptr_birdview_thread);
  sptr_birdview_thread->load_weights_and_masks("./weights.png", "./masks.png");
  sptr_birdview_thread->start();
  while (true) {
    cv::Mat img;
    cv::resize(sptr_birdview_thread->get_image(),img,cv::Size(300,400));
    cv::imshow("birdview", img);
    int key = cv::waitKey(1) & 0xFF;
    if (key == int('q')) {
      break;
    }
    for (std::shared_ptr<CaptureThread> sptr_capture_thread : capture_threads) {
      std::cout<<std::string("camera ") + std::to_string(sptr_capture_thread->get_device_id()) + std::string(" fps: ") + std::to_string(sptr_capture_thread->get_stat_data().average_fps_);
    }
    for (std::shared_ptr<CameraProcessingThread> sptr_process_thread : process_threads) {
      std::cout<<std::string("process ") + std::to_string(sptr_process_thread->get_device_id()) + std::string(" fps: ") + std::to_string(sptr_process_thread->get_stat_data().average_fps_);
    }
    std::cout<<std::string("birdview fps: ") + std::to_string(sptr_birdview_thread->get_stat_data().average_fps_);
  }
  for (std::shared_ptr<CameraProcessingThread> sptr_process_thread : process_threads) {
    sptr_process_thread->stop();
  }
  for (std::shared_ptr<CaptureThread> sptr_capture_thread : capture_threads) {
    sptr_capture_thread->stop();
    sptr_capture_thread->disconnect_camera();
  }
  return 0;
}