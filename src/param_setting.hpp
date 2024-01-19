#ifndef SVS_PARAM_SETTING_HPP
#define SVS_PARAM_SETTING_HPP

#include <map>
#include <string>
#include <vector>
#include <unordered_set>
#include <opencv2/opencv.hpp>

namespace SVS {
class Settings{
  public:
  Settings() {
    cv::resize(car_image, car_image, cv::Size(xr - xl, yb - yt), 0, 0, cv::INTER_LINEAR);
  }
  std::unordered_set<std::string> camera_names{"front", "back", "left", "right"};
  int shift_w{300};
  int shift_h{300};
  int inn_shift_w{20};
  int inn_shift_h{50};
  int total_w{600+2*shift_w};
  int total_h{1000+2*shift_h};
  int xl{shift_w + 180 + inn_shift_w};
  int xr{total_w - xl};
  int yt{shift_h + 200 + inn_shift_h};
  int yb{total_h - yt};
  std::map<std::string,cv::Size> project_shapes{
    {"front",{total_w,yt}},
    {"back",{total_w,yt}},
    {"left",{total_h,xl}},
    {"right",{total_h,xl}}};
  std::map<std::string,std::vector<std::pair<int,int>>> project_keypoints{
    {"front",{{shift_w + 120, shift_h},{shift_w + 480, shift_h},{shift_w + 120, shift_h + 160},{shift_w + 480, shift_h + 160}}},
    {"back", {{shift_w + 120, shift_h},{shift_w + 480, shift_h},{shift_w + 120, shift_h + 160},{shift_w + 480, shift_h + 160}}},
    {"left", {{shift_h + 280, shift_w},{shift_h + 840, shift_w},{shift_h + 280, shift_w + 160},{shift_h + 840, shift_w + 160}}},
    {"right",{{shift_h + 160, shift_w},{shift_h + 720, shift_w},{shift_h + 160, shift_w + 160},{shift_h + 720, shift_w + 160}}}};
  cv::Mat car_image = cv::imread(".data/images/car.png", cv::IMREAD_COLOR);
};
static Settings static_const_settings = Settings();
} //namespace SVS

#endif //SVS_PARAM_SETTING_HPP