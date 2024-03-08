#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <assert.h> 
#include <sys/stat.h>
#include <opencv2/opencv.hpp>
#include "capture_thread.hpp"
#include "imagebuffer.hpp"
#include "utils.hpp"

using namespace SVS;

std::string target_dir = "./data/yamls";
std::string default_param_file = target_dir + "/camera_params.yaml";

int main(int argc, char *argv[]) {
  int camera_device_id{0};
  // size of the calibrate chessboard pattern
  cv::Size chess_size{9,6};
  // resolution of the camera image
  cv::Size resolution{640,480};
  // use every nth frame in the video
  int frame_step{20};
  std::string output_param_file{default_param_file};
  // whether camera is fisheye
  bool is_fisheye {true};
  // flip method of camera
  FlipMethod flip_method{FlipMethod::Identity};
  // whether to use gstreamer for the camera capture
  bool no_gst{true};
  struct stat sb;
  if (stat(target_dir.c_str(), &sb) != 0) {
    mkdir(target_dir.c_str(),0755);
  }
  std::string text1{"press c to calibrate"};
  std::string text2{"press q to quit"};
  std::string text3{"device: "};
  text3 += std::to_string(camera_device_id);
  cv::HersheyFonts font = cv::HersheyFonts::FONT_HERSHEY_SIMPLEX;
  double fontscale = 0.6;
  int W{resolution.width};
  int H{resolution.height};
  std::vector<cv::Point3d> objp;
  for(int i = 0; i<H; i++){
      for(int j = 0; j<W; j++){
        objp.push_back(cv::Point3f(j,i,0));
      }
    }
  // 3d points in real world
  std::vector<std::vector<cv::Point3d>> objpoints;
  // 2d points in image plane
  std::vector<std::vector<cv::Point2d>> imgpoints;
  std::shared_ptr<CaptureThread> sptr_capture_thread = std::make_shared<CaptureThread>(camera_device_id, flip_method, true, cv::CAP_GSTREAMER, resolution, !no_gst);
  std::shared_ptr<MultiBufferManager<ImageFrame,CaptureThread>> sptr_multi_buffer_manager = std::make_shared<MultiBufferManager<ImageFrame,CaptureThread>>();
  sptr_multi_buffer_manager->create_buffer_and_bind_thread(sptr_capture_thread, 8, true);
  if (sptr_capture_thread->connect_camera()) {
    sptr_capture_thread->start();
  } else {
    throw std::runtime_error("cannot open device");
  }
  bool quit = false;
  bool do_calib = false;
  int i = -1;
  cv::Mat img;
  while (true) {
    i++;
    img = sptr_multi_buffer_manager->get(camera_device_id).image_.clone();
    if (i % frame_step == 0) {
      std::cout<<"searching for chessboard corners in frame " + std::to_string(i) + "..."<<std::endl;
      cv::Mat gray;
      cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
      std::vector<cv::Point2d> corners;
      bool found = cv::findChessboardCorners(gray,chess_size,corners,cv::CALIB_CB_ADAPTIVE_THRESH + cv::CALIB_CB_NORMALIZE_IMAGE + cv::CALIB_CB_FILTER_QUADS);
      if (found) {
        cv::cornerSubPix(gray, corners, cv::Size(5, 5), cv::Size(-1, -1), cv::TermCriteria(cv::TermCriteria::Type::EPS + cv::TermCriteria::Type::COUNT, 30, 0.01));
        std::cout<<"OK."<<std::endl;
        imgpoints.push_back(corners);
        objpoints.push_back(objp);
        cv::drawChessboardCorners(img, chess_size, corners, found);
      }
      cv::putText(img, text1, cv::Point(20, 70),  font, fontscale,  cv::Scalar(255, 200, 0), 2);
      cv::putText(img, text2, cv::Point(20, 110), font, fontscale,  cv::Scalar(255, 200, 0), 2);
      cv::putText(img, text3, cv::Point(20, 30),  font, fontscale,  cv::Scalar(255, 200, 0), 2);
      cv::imshow("corners", img);
      // & 0xFF is to only get lower 8 bits of waiKey results.
      int key = (cv::waitKey(1) & 0xFF);
      if (key == int('c')) {
        std::cout<<"Performing calibration..."<<std::endl;
        int N_OK = objpoints.size();
        if (N_OK < 12) {
          std::cout<<"Less than 12 corners, " + std::to_string(N_OK) + " detected, calibration failed"<<std::endl;
          continue;
        } else {
          do_calib = true;
          break;
        }
      } else if (key == int('q')) {
        quit = true;
        break;
      }
    }
  }
  if (quit) {
    sptr_capture_thread->stop();
    sptr_capture_thread->disconnect_camera();
    cv::destroyAllWindows();
  }
  if (do_calib) {
    int N_OK = objpoints.size();
    cv::Mat K = cv::Mat::zeros(3,3,CV_64FC1);
    cv::Mat D = cv::Mat::zeros(4,1,CV_64FC1);
    std::vector<cv::Mat> rvecs;
    for (int i=0;i<N_OK;i++) {
      rvecs.push_back(cv::Mat::zeros(1,1,CV_64FC3));
    }
    std::vector<cv::Mat> tvecs;
    for (int i=0;i<N_OK;i++) {
      tvecs.push_back(cv::Mat::zeros(1,1,CV_64FC3));
    }
    double ret = 0.0;
    if (is_fisheye){
      ret = cv::fisheye::calibrate(objpoints,imgpoints,cv::Size(W,H),K,D,rvecs,tvecs,cv::fisheye::CALIB_RECOMPUTE_EXTRINSIC + cv::fisheye::CALIB_CHECK_COND + cv::fisheye::CALIB_FIX_SKEW,cv::TermCriteria(cv::TermCriteria::Type::EPS + cv::TermCriteria::Type::MAX_ITER, 30, 1e-6));
    } else {
      cv::Mat camera_matrix, dist_coeffs, std_deviations_intrinsics, std_deviations_extrinsics, perview_errors;
      ret = cv::calibrateCamera(objpoints,imgpoints,cv::Size(W,H),camera_matrix, dist_coeffs, rvecs, tvecs, std_deviations_intrinsics, std_deviations_extrinsics, perview_errors);	
    }
    if (ret) {
      cv::FileStorage fs{output_param_file, cv::FileStorage::WRITE};
      fs.write("resolution", cv::Mat{W,H});
      fs.write("camera_matrix", K);
      fs.write("dist_coeffs", D);
      fs.release();
      std::cout<<"successfully saved camera data"<<std::endl;
      cv::putText(img, "Success!", cv::Point(220, 240), font, 2, cv::Scalar(0, 0, 255), 2);
    }
  }
}