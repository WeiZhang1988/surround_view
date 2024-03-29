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

int main(int argc, char *argv[]){
  std::vector<std::string> names = static_settings.camera_names;
  std::vector<std::string> images, yamls;
  for (std::string name : names) {
    images.push_back(std::string("./data/images/") + name + std::string(".png"));
    yamls.push_back(std::string("./data/yamls/") + name + std::string(".yaml"));
  }
  assert(images.size() == yamls.size());
  std::vector<std::shared_ptr<CameraModel>> camera_models;
  std::vector<cv::Mat> projected;
  for (int i=0;i<names.size();i++){
    std::shared_ptr<CameraModel> sptr_camera = std::make_shared<CameraModel>(yamls[i],names[i]);
    cv::Mat img = cv::imread(images[i]);
    img = sptr_camera->undistort(img);
    img = sptr_camera->project(img);
    img = sptr_camera->flip(img);
    camera_models.push_back(sptr_camera);
    projected.push_back(img.clone());
  }
  std::shared_ptr<BirdView> sptr_birdview = std::make_shared<BirdView>();
  cv::Mat Gmat, Mmat;
  sptr_birdview->get_weights_and_masks(projected,Gmat,Mmat);
  sptr_birdview->update_frames(projected);
  sptr_birdview->make_luminance_balance();
  sptr_birdview->stitch_all_parts();
  sptr_birdview->make_white_balance();
  sptr_birdview->copy_car_image();
  int ret = display_image("BirdView Result", sptr_birdview->get_image());
  if (ret > 0){
    bool x = cv::imwrite("./data/weights.png",Gmat * 255);
    bool y = cv::imwrite("./data/masks.png",Mmat);
  }
  return 0;
}