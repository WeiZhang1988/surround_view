#ifndef SVS_FISHEYE_CAMERA_HPP
#define SVS_FISHEYE_CAMERA_HPP

#include <unistd.h>
#include <string>
#include <stdexcept>
#include <opencv2/opencv.hpp>
#include "param_setting.hpp"

namespace SVS {
class FisheyeCameraModel {
  public:
  FisheyeCameraModel(std::string _camera_param_file, std::string _camera_name) {
    if ( access( _camera_param_file.c_str(), F_OK ) != -1 ) {
      throw std::runtime_error("Cannot find camera param file");
    }
    if (settings_.camera_names.find(_camera_name) == settings_.camera_names.end()) {
      throw std::runtime_error("Unknown camera name: " + _camera_name);
    }
    camera_param_file_ = _camera_param_file;
    camera_name_ = _camera_name;
    scale_xy_ = cv::Mat{1.0,1.0};
    shift_xy_ = cv::Mat{0,0};
    project_shape_ = settings_.project_shapes[camera_name_];
    load_camera_params();
  }
  void load_camera_params() {
    cv::FileStorage fs{camera_param_file_, cv::FileStorage::READ};
    fs["camera_matrix"]>>camera_matrix_;
    fs["dist_coeffs"]>>dist_coeffs_;
    fs["resolution"]>>resolution_;
    fs["scale_xy"]>>scale_xy_;
    fs["shift_xy"]>>shift_xy_;
    fs["project_matrix"]>>project_matrix_;
    fs.release();
    update_distort_maps();
  }
  void update_distort_maps(){
    cv::Mat new_camera_matrix = camera_matrix_.clone();
    new_camera_matrix.at(0,0) *= scale_xy_[0];
    new_camera_matrix.at(1,1) *= scale_xy_[1];
    new_camera_matrix.at(0,2) *= shift_xy_[0];
    new_camera_matrix.at(1,2) *= shift_xy_[1];
    cv::initUndistortRectifyMap(camera_matrix_, dist_coeffs_, cv::Mat::eye(3,3,CV_64F),new_camera_matrix,undistort_map_x_,undistort_map_y_);
  }
  protected:
  Settings settings_;
  std::string camera_param_file_, camera_name_;
  cv::Mat scale_xy_, shift_xy_, undistort_map_x_, undistort_map_y_, project_matrix_, camera_matrix_, dist_coeffs_;
  cv::Size project_shape_, resolution_;
};

} //namespace SVS

#endif //SVS_FISHEYE_CAMERA_HPP