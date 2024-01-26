#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <opencv2/opencv.hpp>
#include "camera_model.hpp"
#include "simple_gui.hpp"
#include "birdview.hpp"
#include "param_setting.hpp"

using namespace SVS;

bool get_projection_map(std::shared_ptr<CameraModel> _sptr_camera, cv::Mat _image) {
  cv::Mat und_image = _sptr_camera->undistort(_image.clone());
  std::string name = _sptr_camera->get_name();
  PointSelector gui = PointSelector(und_image.clone(),name);
  cv::waitKey(0);
  std::vector<cv::Point> dst_points = static_settings.project_keypoints[name];
  bool choice = gui.loop();
  if (choice) {
    std::vector<cv::Point> src_points = gui.get_keypoints();
    cv::Mat src = cv::Mat(src_points);
    src.convertTo(src,CV_32FC1);
    cv::Mat dst = cv::Mat(dst_points);
    dst.convertTo(dst,CV_32FC1);
    cv::Mat project_matrix = cv::getPerspectiveTransform(src, dst);
    project_matrix.convertTo(project_matrix,CV_64FC1);
    _sptr_camera->set_project_matrix(project_matrix);
    cv::Mat proj_image = _sptr_camera->project(und_image.clone());
    int ret = display_image("Bird's View", proj_image);
    if (ret > 0) {
      return true;
    } else if (ret < 0){
      cv::destroyAllWindows();
    }
  }
  return false;
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    throw std::runtime_error("usage: ./run_get_projection_maps [camera name, one in [front, back, left, right]]");
    return -1;
  }
  std::string camera_name = argv[1];
  if (std::find(static_settings.camera_names.begin(),static_settings.camera_names.end(),camera_name) == static_settings.camera_names.end()) {
    throw std::runtime_error("wrong camera name");
    return -1;
  } 
  std::string camera_file = std::string("./data/yamls/") + camera_name + std::string(".yaml");
  std::string image_file = std::string("./data/images/") + camera_name + std::string(".png");
  cv::Mat image = cv::imread(image_file,cv::IMREAD_UNCHANGED);
  std::shared_ptr<CameraModel> sptr_camera = std::make_shared<CameraModel>(camera_file, camera_name);
  sptr_camera->set_scale_and_shift(static_settings.scale.clone(), static_settings.shift.clone());
  if (get_projection_map(sptr_camera,image.clone())) {
    std::cout<<"saving projection matrix to yaml"<<std::endl;
    sptr_camera->save_data();
  } else {
    std::cout<<"failed to compute the projection map"<<std::endl;
  }
  return 0;
}