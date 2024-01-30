#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <chrono>
#include <ctime>
#include <sys/stat.h>
#include <opencv2/opencv.hpp>
#include "capture_thread.hpp"
#include "imagebuffer.hpp"
#include "utils.hpp"

using namespace SVS;

std::vector<int> get_cam_vec(std::vector<int> _cam_id_vec) {
  std::vector<int> cam_vec;
  for (const int &i : _cam_id_vec) {
    cv::VideoCapture cap = cv::VideoCapture(i);
    cv::Mat frame;
    if (cap.read(frame)) {
      cam_vec.push_back(i);
    } else {
      continue;
    }
    cap.release();
  }
  return cam_vec;
}

void show_cam_img(std::vector<cv::VideoCapture> _cap_vec, std::vector<int> _cam_vec) {
  std::cout<<"INFO: Press 'q' to quit! Press 's' to save a picture, 'n' to change to next camera device!"<<std::endl;
  int idx = 0;
  while (true) {
    cv::VideoCapture cap_device = _cap_vec[idx];
    cv::Mat frame;
    bool ret = cap_device.read(frame);
    if (ret) {
      cv::imshow("video",frame);
    } else {
      std::cout<<"Error: failed reading frame"<<std::endl;
    }
    int c = cv::waitKey(1);
    if (c==int('q')) {
      break;
    }
    if (c==int('n')) {
      idx++;
      if (idx >= _cap_vec.size()) {
        idx = 0;
      } 
      continue;
    }
    if (ret && (c==int('s'))) {
      auto now = std::chrono::system_clock::now();
      std::time_t now_c = std::chrono::system_clock::to_time_t(now);
      std::string name = "video" + std::to_string(_cam_vec[idx]) + "_" + std::ctime(&now_c) + ".png";
      cv::imwrite(name,frame);
    }
  }
  cv::destroyAllWindows();
}

std::vector<cv::VideoCapture> init_cap_vec(std::vector<int> _cam_vec, cv::Size _resolution = cv::Size(640,480)) {
  std::vector<cv::VideoCapture> cap_vec;
  for (const int &i : _cam_vec) {
    cv::VideoCapture cap = cv::VideoCapture(i);
    cap.set(cv::CAP_PROP_FRAME_WIDTH,  _resolution.width);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, _resolution.height);
    cap.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'));
    cv::Mat frame;
    if (cap.read(frame)) {
    } else {
    }
    cap_vec.push_back(cap);
  }
  return cap_vec;
}

void deinit_cap_vec(std::vector<cv::VideoCapture> &_cap_vec) {
  for (cv::VideoCapture &cap : _cap_vec) {
    cap.release();
  }
}

void show_cameras(std::vector<int> _cam_id_vec){
  std::string err_msg;
  std::vector<int> cam_vec;
  if (_cam_id_vec.empty()){
    std::cout<<"Start to search all available camera devices, please wait... "<<std::endl;
    std::vector<int> cam_id_vec;
    for (int i=0;i<25;i++) {
      cam_id_vec.push_back(i);
    }
    cam_vec = get_cam_vec(cam_id_vec);
    err_msg = "cannot find any video device!";
  } else {
    cam_vec = get_cam_vec(_cam_id_vec);
    err_msg = std::string("cannot find available video device in the list")+std::string("\nPlease check the video devices in /dev/v4l/by-path/ folder!");
  }
  if (cam_vec.size()<1) {
    std::cout<<"ERROR: " + err_msg;
    return;
  }
  std::vector<cv::VideoCapture> cap_vec = init_cap_vec(cam_vec);
  show_cam_img(cap_vec,cam_vec);
  deinit_cap_vec(cap_vec);
}

int main(int argc, char *argv[]) {
  std::vector<int> cam_vec{0};
  show_cameras(cam_vec);
  return 0;
}