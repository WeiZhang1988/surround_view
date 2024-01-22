#ifndef SVS_BIRDVIEW_HPP
#define SVS_BIRDVIEW_HPP

#include <cmath>
#include <vector>
#include <memory>
#include <opencv2/opencv.hpp>
#include "structures.hpp"
#include "param_setting.hpp"
#include "base_thread.hpp"
#include "imagebuffer.hpp"
#include "process_thread.hpp"
#include "utils.hpp"

namespace SVS{
cv::Mat FI(cv::Mat _front_image) {
  return _front_image(cv::Range::all(),cv::Range(0,static_settings.xl)).clone();
}
cv::Mat FII(cv::Mat _front_image) {
  return _front_image(cv::Range::all(),cv::Range(static_settings.xr,_front_image.cols)).clone();
}
cv::Mat FM(cv::Mat _front_image) {
  return _front_image(cv::Range::all(),cv::Range(static_settings.xl,static_settings.xr)).clone();
}
cv::Mat BIII(cv::Mat _back_image) {
  return _back_image(cv::Range::all(),cv::Range(0,static_settings.xl)).clone();
}
cv::Mat BIV(cv::Mat _back_image) {
  return _back_image(cv::Range::all(),cv::Range(static_settings.xr,_back_image.cols)).clone();
}
cv::Mat BM(cv::Mat _back_image) {
  return _back_image(cv::Range::all(),cv::Range(static_settings.xl,static_settings.xr)).clone();
}
cv::Mat LI(cv::Mat _left_image) {
  return _left_image(cv::Range(0,static_settings.yt),cv::Range::all()).clone();
}
cv::Mat LIII(cv::Mat _left_image) {
  return _left_image(cv::Range(static_settings.yb,_left_image.rows),cv::Range::all()).clone();
}
cv::Mat LM(cv::Mat _left_image) {
  return _left_image(cv::Range(static_settings.yb,static_settings.yt),cv::Range::all()).clone();
}
cv::Mat RII(cv::Mat _right_image) {
  return _right_image(cv::Range(0,static_settings.yt),cv::Range::all()).clone();
}
cv::Mat RIV(cv::Mat _right_image) {
  return _right_image(cv::Range(static_settings.yb,_right_image.rows),cv::Range::all()).clone();
}
cv::Mat RM(cv::Mat _right_image) {
  return _right_image(cv::Range(static_settings.yt,static_settings.yb),cv::Range::all()).clone();
}

class BirdView : public BaseThread, std::enable_shared_from_this<BirdView> {
  public:
  BirdView(bool _drop_if_full = true, int _buffer_size = 8) : drop_if_full_(_drop_if_full){
    sptr_buffer_ = std::make_shared<Buffer<cv::Mat>>(_buffer_size);
  }
  cv::Mat get() {
    return sptr_buffer_->get().clone();
  }
  void update_frames(std::vector<cv::Mat> _vec_images) {
    vec_images_ = _vec_images;
  }
  void load_weights_and_masks(std::string _weights_image, std::string _masks_iamge) {
    cv::Mat GMat = cv::imread(_weights_image, cv::IMREAD_UNCHANGED);
    cv::cvtColor(GMat, GMat, cv::COLOR_BGRA2RGBA);
    GMat = GMat / 255.0;
    cv::Mat result;
    vec_weights_.clear();
    for (int k=0; k<4; k++) {
      std::vector<cv::Mat> vec{GMat(cv::Range::all(),cv::Range::all(),k),GMat(cv::Range::all(),cv::Range::all(),k),GMat(cv::Range::all(),cv::Range::all(),k)};
      cv::merge(vec,result);
      vec_weights_.push(result.clone());
    }
    cv::Mat Mmat = cv::imread(_masks_iamge, cv::IMREAD_UNCHANGED);
    cv::cvtColor(GMat, GMat, cv::COLOR_BGRA2RGBA);
    Mmat = convert_binary_to_bool(Mmat);
    vec_masks_.clear();
    for (int k=0; k<4; k++) {
      vec_masks_.push(MMat(cv::Range::all(),cv::Range::all(),k).clone());
    }
  }
  cv::Mat merge(cv::Mat _imA, cv::Mat _imB, int k) {
    return (_imA * vec_weights[k] + _imB * (1 - vec_weights[k])).clone();
  }
  void stich_all_parts() {
    cv::Mat front = vec_images[0].clone();
    cv::Mat back  = vec_images[1].clone();
    cv::Mat left  = vec_images[2].clone();
    cv::Mat right = vec_images[3].clone();
    F_ = FM(front_);
    B_ = BM(back_);
    L_ = LM(left_);
    R_ = RM(right_);
    FL_ = merge(FI(front), LI(left), 0);
    FR_ = merge(FII(front), RII(right), 1);
    BL_ = merge(BIII(back), LIII(left), 2);
    BR_ = merge(BIV(back), RIV(right), 3);
  }
  void copy_car_image() {
    C_ = car_image_.clone();
  }
  double tune(double _x) {
    if (_x > 1.0) {
      return x * std::exp(1.0 - x, 0.5);
    } else {
      return x * std::exp(1.0 - x, 0.8);
    }
  }
  std::shared_ptr<BirdView> make_luminance_balance() {
    //stop here
    return this->shared_from_this();
  }
  protected:
  cv::Mat FL() const {
    return image_(cv::Range(0,static_settings.yt),cv::Range(0,static_settings.xl)).clone();
  }
  cv::Mat F() const {
    return image_(cv::Range(0,static_settings.yt),cv::Range(static_settings.xl,static_settings.xr)).clone();
  }
  cv::Mat FR() const {
    return image_(cv::Range(0,static_settings.yt),cv::Range(static_settings.xr,image_.cols)).clone();
  }
  cv::Mat BL() const {
    return image_(cv::Range(static_settings.yb,image_.rows),cv::Range(0,static_settings.xl)).clone();
  }
  cv::Mat B() const {
    return image_(cv::Range(static_settings.yb,image_.rows),cv::Range(static_settings.xl,static_settings.xr)).clone();
  }
  cv::Mat BR() const {
    return image_(cv::Range(static_settings.yb,image_.rows),cv::Range(static_settings.xr,image_.cols)).clone();
  }
  cv::Mat L() const {
    return image_(cv::Range(static_settings.yt,static_settings.yb),cv::Range(0,static_settings.xl)).clone();
  }
  cv::Mat R() const {
    return image_(cv::Range(static_settings.yt,static_settings.yb),cv::Range(static_settings.xl,image_.cols)).clone();
  }
  cv::Mat C() const {
    return image_(cv::Range(static_settings.yt,static_settings.yb),cv::Range(static_settings.xl,static_settings.xr)).clone();
  }
  bool drop_if_full = false;
  std::shared_ptr<Buffer<cv::Mat>> sptr_buffer_ = std::shared_ptr<Buffer<cv::Mat>>(nullptr);
  std::shared_ptr<ProjectedImageBufferManager> sptr_proc_buffer_manager_ = std::shared_ptr<ProjectedImageBufferManager>(nullptr);
  cv::Mat car_image_ = static_settings.car_image.clone();
  cv::Mat image_ = cv::Mat::zeros(static_settings.total_h, static_settings.total_w);
  std::vector<cv::Mat> vec_images_, vec_weights_, vec_masks_;
  cv::Mat F_, B_, L_, R_, FL_, FR_, BL_, BR_, C_;
};
} //namespace SVS

#endif //SVS_BIRDVIEW_HPP