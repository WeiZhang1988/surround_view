#ifndef SVS_FISHEYE_CAMERA_HPP
#define SVS_FISHEYE_CAMERA_HPP

#include <unistd.h>
#include <string>
#include <memory>
#include <stdexcept>
#include <opencv2/opencv.hpp>
#include "param_setting.hpp"

namespace SVS {
class CameraModel : public std::enable_shared_from_this<CameraModel> {
  public:
  CameraModel(std::string _camera_param_file, std::string _camera_name) {
    if ( access( _camera_param_file.c_str(), F_OK ) != -1 ) {
      throw std::runtime_error("Cannot find camera param file");
    }
    if (static_settings.camera_names.find(_camera_name) == static_settings.camera_names.end()) {
      throw std::runtime_error("Unknown camera name: " + _camera_name);
    }
    camera_param_file_ = _camera_param_file;
    camera_name_ = _camera_name;
    scale_xy_ = cv::Mat{1.0,1.0};
    shift_xy_ = cv::Mat{0,0};
    project_shape_ = static_settings.project_shapes[camera_name_];
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
  std::shared_ptr<CameraModel> update_distort_maps(){
    cv::Mat new_camera_matrix = camera_matrix_.clone();
    new_camera_matrix.at<double>(0,0) *= scale_xy_.at<double>(0);
    new_camera_matrix.at<double>(1,1) *= scale_xy_.at<double>(1);
    new_camera_matrix.at<double>(0,2) *= shift_xy_.at<double>(0);
    new_camera_matrix.at<double>(1,2) *= shift_xy_.at<double>(1);
    cv::initUndistortRectifyMap(camera_matrix_, dist_coeffs_, cv::Mat::eye(3,3,CV_64F),new_camera_matrix, resolution_, CV_16SC2, undistort_map_x_,undistort_map_y_);
    return this->shared_from_this();
  }
  std::shared_ptr<CameraModel> set_scale_and_shift(cv::Mat scale_xy = cv::Mat{1.0,1.0}, cv::Mat shift_xy = cv::Mat{0,0}) {
    scale_xy_ = scale_xy.clone();
    shift_xy_ = shift_xy.clone();
    update_distort_maps();
    return this->shared_from_this();
  }
  cv::Mat undistort(cv::Mat image) {
    cv::Mat result;
    cv::remap(image.clone(),result,undistort_map_x_,undistort_map_y_,cv::INTER_LINEAR,cv::BORDER_CONSTANT,cv::Scalar());
    return result.clone();
  }
  cv::Mat project(cv::Mat image) {
    cv::Mat result;
    cv::warpPerspective(image,result,project_matrix_,project_shape_);
    return result.clone();
  }
  cv::Mat flip(cv::Mat image) {
    if (camera_name_ == "front") {
      return image.clone();
    } else if (camera_name_ == "back") {
      cv::Mat result;
      cv::flip(image.clone(),result,1);
      return result.clone();
    } else if (camera_name_ == "left") {
      cv::Mat result1, result2;
      cv::transpose(image.clone(),result1);
      cv::flip(result1.clone(),result2,0);
      return result2.clone();
    } else if (camera_name_ == "right") {
      cv::Mat result1, result2;
      cv::transpose(image.clone(),result1);
      cv::flip(result1.clone(),result2,-1);
      return result2.clone();
    } else {
      throw std::runtime_error("No such camera found.");
    }
  }
  void save_data() {
    cv::FileStorage fs{camera_param_file_, cv::FileStorage::WRITE};
    fs.write("camera_matrix",camera_matrix_);
    fs.write("dist_coeffs",dist_coeffs_);
    fs.write("resolution",cv::Mat{resolution_.width,resolution_.height});
    fs.write("scale_xy",scale_xy_);
    fs.write("shift_xy",shift_xy_);
    fs.write("project_matrix",project_matrix_);
    fs.release();
  }
  protected:
  std::string camera_param_file_, camera_name_;
  cv::Mat scale_xy_, shift_xy_, undistort_map_x_, undistort_map_y_, project_matrix_, camera_matrix_, dist_coeffs_;
  cv::Size project_shape_, resolution_;
};

} //namespace SVS

#endif //SVS_FISHEYE_CAMERA_HPP