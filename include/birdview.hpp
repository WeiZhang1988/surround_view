#ifndef SVS_BIRDVIEW_HPP
#define SVS_BIRDVIEW_HPP

#include <cmath>
#include <vector>
#include <memory>
#include <mutex>
#include <stdexcept>
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
  return _left_image(cv::Range(static_settings.yt,static_settings.yb),cv::Range::all()).clone();
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

class BirdView : public BaseThread {
  public:
  BirdView(bool _drop_if_full = true, int _buffer_size = 8) : drop_if_full_(_drop_if_full){
    sptr_buffer_ = std::make_shared<Buffer<cv::Mat>>(_buffer_size);
  }
  cv::Mat get() {
    return sptr_buffer_->get().clone();
  }
  cv::Mat get_image() {
    return image_.clone();
  }
  void update_frames(std::vector<cv::Mat> _vec_images) {
    vec_images_ = _vec_images;
  }
  void load_weights_and_masks(std::string _weights_image, std::string _masks_iamge) {
    cv::Mat GMat = cv::imread(_weights_image, cv::IMREAD_UNCHANGED);
    cv::cvtColor(GMat, GMat, cv::COLOR_BGRA2RGBA);
    GMat.convertTo(GMat,CV_64F);
    GMat = GMat / 255.0;
    cv::Mat result;
    vec_weights_.clear();
    std::vector<cv::Mat> channels(3);
    cv::split(GMat, channels);
    for (int k=0; k<4; k++) {
      std::vector<cv::Mat> vec{channels[k],channels[k],channels[k]};
      cv::merge(vec,result);
      vec_weights_.push_back(result.clone());
    }
    cv::Mat Mmat = cv::imread(_masks_iamge, cv::IMREAD_UNCHANGED);
    cv::cvtColor(Mmat, Mmat, cv::COLOR_BGRA2RGBA);
    Mmat = static_utils.convert_binary_to_bool(Mmat);
    vec_masks_.clear();
    cv::split(Mmat, vec_masks_);
  }
  cv::Mat merge(cv::Mat _imA, cv::Mat _imB, int k) {
    cv::Mat tmpA, tmpB, result;
    _imA.convertTo(tmpA, CV_64F);
    _imB.convertTo(tmpB, CV_64F);
    result = tmpA.mul(vec_weights_[k].clone()) + tmpB.mul(1 - vec_weights_[k].clone());
    result.convertTo(result, CV_8U);
    return result.clone();
  }
  void stitch_all_parts() {
    cv::Mat front = vec_images_[0].clone();
    cv::Mat back  = vec_images_[1].clone();
    cv::Mat left  = vec_images_[2].clone();
    cv::Mat right = vec_images_[3].clone();
    FM(front).copyTo(F());
    BM(back).copyTo(B());
    LM(left).copyTo(L());
    RM(right).copyTo(R());
    merge(FI(front), LI(left), 0).copyTo(FL());
    merge(FII(front), RII(right), 1).copyTo(FR());
    merge(BIII(back), LIII(left), 2).copyTo(BL());
    merge(BIV(back), RIV(right), 3).copyTo(BR());
  }
  void copy_car_image() {
    car_image_.copyTo(C());
  }
  double tune(double _x) {
    if (_x > 1.0) {
      return std::exp((1.0 - _x) * 0.5);
    } else {
      return std::exp((1.0 - _x) * 0.8);
    }
  }
  void make_luminance_balance() {
    cv::Mat front = vec_images_[0].clone();
    cv::Mat back  = vec_images_[1].clone();
    cv::Mat left  = vec_images_[2].clone();
    cv::Mat right = vec_images_[3].clone();
    cv::Mat m1 = vec_masks_[0].clone();
    cv::Mat m2 = vec_masks_[1].clone();
    cv::Mat m3 = vec_masks_[2].clone();
    cv::Mat m4 = vec_masks_[3].clone();
    std::vector<cv::Mat> Fv, Bv, Lv, Rv;
    cv::split(front,Fv); 
    cv::Mat Fb = Fv[0].clone();
    cv::Mat Fg = Fv[1].clone();
    cv::Mat Fr = Fv[2].clone();
    cv::split(back,Bv);
    cv::Mat Bb = Bv[0].clone();
    cv::Mat Bg = Bv[1].clone();
    cv::Mat Br = Bv[2].clone();
    cv::split(left,Lv);
    cv::Mat Lb = Lv[0].clone();
    cv::Mat Lg = Lv[1].clone();
    cv::Mat Lr = Lv[2].clone();
    cv::split(right,Rv);
    cv::Mat Rb = Rv[0].clone();
    cv::Mat Rg = Rv[1].clone();
    cv::Mat Rr = Rv[2].clone();
    double a1 = static_utils.mean_luminance_ratio(RII(Rb), FII(Fb), m2);
    double a2 = static_utils.mean_luminance_ratio(RII(Rg), FII(Fg), m2);
    double a3 = static_utils.mean_luminance_ratio(RII(Rr), FII(Fr), m2);
    double b1 = static_utils.mean_luminance_ratio(BIV(Bb), RIV(Rb), m4);
    double b2 = static_utils.mean_luminance_ratio(BIV(Bg), RIV(Rg), m4);
    double b3 = static_utils.mean_luminance_ratio(BIV(Br), RIV(Rr), m4);
    double c1 = static_utils.mean_luminance_ratio(LIII(Lb), BIII(Bb), m3);
    double c2 = static_utils.mean_luminance_ratio(LIII(Lg), BIII(Bg), m3);
    double c3 = static_utils.mean_luminance_ratio(LIII(Lr), BIII(Br), m3);
    double d1 = static_utils.mean_luminance_ratio(FI(Fb), LI(Lb), m1);
    double d2 = static_utils.mean_luminance_ratio(FI(Fg), LI(Lg), m1);
    double d3 = static_utils.mean_luminance_ratio(FI(Fr), LI(Lr), m1);
    double t1, t2, t3;
    t1 = std::pow(a1 * b1 * c1 * d1,0.25f);
    t2 = std::pow(a2 * b2 * c2 * d2,0.25f);
    t3 = std::pow(a3 * b3 * c3 * d3,0.25f);
    double tmp1, tmp2, tmp3;
    tmp1 = std::pow(d1 / a1,0.5f);
    tmp2 = std::pow(d2 / a2,0.5f);
    tmp3 = std::pow(d3 / a3,0.5f);
    double x1 = t1 / tmp1;
    double x2 = t2 / tmp2;
    double x3 = t3 / tmp3;
    x1 = tune(x1);
    x2 = tune(x2);
    x3 = tune(x3);
    Fb = static_utils.adjust_luminance(Fb,x1);
    Fg = static_utils.adjust_luminance(Fg,x2);
    Fr = static_utils.adjust_luminance(Fr,x3);
    tmp1 = std::pow(b1 / c1,0.5);
    tmp2 = std::pow(b2 / c2,0.5);
    tmp3 = std::pow(b3 / c3,0.5);
    double y1 = t1 / tmp1;
    double y2 = t2 / tmp2;
    double y3 = t3 / tmp3;
    y1 = tune(y1);
    y2 = tune(y2);
    y3 = tune(y3);
    Bb = static_utils.adjust_luminance(Bb,y1);
    Bg = static_utils.adjust_luminance(Bg,y2);
    Br = static_utils.adjust_luminance(Br,y3);
    tmp1 = std::pow(c1 / d1,0.5);
    tmp2 = std::pow(c2 / d2,0.5);
    tmp3 = std::pow(c3 / d3,0.5);
    double z1 = t1 / tmp1;
    double z2 = t2 / tmp2;
    double z3 = t3 / tmp3;
    z1 = tune(z1);
    z2 = tune(z2);
    z3 = tune(z3);
    Lb = static_utils.adjust_luminance(Lb,z1);
    Lg = static_utils.adjust_luminance(Lg,z2);
    Lr = static_utils.adjust_luminance(Lr,z3);
    tmp1 = std::pow(a1 / b1,0.5);
    tmp2 = std::pow(a2 / b2,0.5);
    tmp3 = std::pow(a3 / b3,0.5);
    double w1 = t1 / tmp1;
    double w2 = t2 / tmp2;
    double w3 = t3 / tmp3;
    w1 = tune(w1);
    w2 = tune(w2);
    w3 = tune(w3);
    Rb = static_utils.adjust_luminance(Rb,w1);
    Rg = static_utils.adjust_luminance(Rg,w2);
    Rr = static_utils.adjust_luminance(Rr,w3);
    cv::Mat imgF, imgB, imgL, imgR;
    cv::merge(std::vector<cv::Mat>{Fb,Fg,Fr},imgF);
    cv::merge(std::vector<cv::Mat>{Bb,Bg,Br},imgB);
    cv::merge(std::vector<cv::Mat>{Lb,Lg,Lr},imgL);
    cv::merge(std::vector<cv::Mat>{Rb,Rg,Rr},imgR);
    vec_images_ = {imgF, imgB, imgL, imgR};
  }
  void get_weights_and_masks(std::vector<cv::Mat> _images, cv::Mat &_out_G, cv::Mat &_out_M) {
    cv::Mat front = _images[0].clone();
    cv::Mat back  = _images[1].clone();
    cv::Mat left  = _images[2].clone();
    cv::Mat right = _images[3].clone();
    cv::Mat G0,M0,G1,M1,G2,M2,G3,M3;
    static_utils.get_weight_mask_matrix(FI(front.clone()), LI(left.clone()), G0, M0);
    static_utils.get_weight_mask_matrix(FII(front.clone()), RII(right.clone()), G1, M1);
    static_utils.get_weight_mask_matrix(BIII(back.clone()), LIII(left.clone()), G2, M2);
    static_utils.get_weight_mask_matrix(BIV(back.clone()), RIV(right.clone()), G3, M3);
    cv::Mat tmp0, tmp1, tmp2, tmp3;
    cv::merge(std::vector<cv::Mat>{G0,G0,G0},tmp0);
    cv::merge(std::vector<cv::Mat>{G1,G1,G1},tmp1);
    cv::merge(std::vector<cv::Mat>{G2,G2,G2},tmp2);
    cv::merge(std::vector<cv::Mat>{G3,G3,G3},tmp3);
    vec_weights_ = std::vector<cv::Mat>{tmp0,tmp1,tmp2,tmp3}; 
    tmp0 = M0 / 255.0;
    tmp0.convertTo(tmp0, CV_8U);
    tmp1 = M1 / 255.0;
    tmp1.convertTo(tmp1, CV_8U);
    tmp2 = M2 / 255.0;
    tmp2.convertTo(tmp2, CV_8U);
    tmp3 = M3 / 255.0;
    tmp3.convertTo(tmp3, CV_8U);
    vec_masks_ = std::vector<cv::Mat>{tmp0,tmp1,tmp2,tmp3}; 
    cv::merge(std::vector<cv::Mat>{G0,G1,G2,G3},_out_G);   
    cv::merge(std::vector<cv::Mat>{M0,M1,M2,M3},_out_M);    
  }
  void make_white_balance() {
    image_ = static_utils.make_white_balance(image_.clone());
  }
  void run() {
    if (sptr_proc_buffer_manager_ == std::shared_ptr<ProjectedImageBufferManager>(nullptr)) {
      std::runtime_error("This thread requires a buffer of projected images to run");
    }
    while(true) {
      {
        std::unique_lock<std::mutex> lock(stop_mutex_);
        if (stopped_) {
          stopped_ = false;
          break;
        }
      }
      processing_time_ = clock_.elapsed();
      clock_.start();
      {
        std::unique_lock<std::mutex> lock(processing_mutex_);
        std::map<int, cv::Mat> tmpMap = sptr_proc_buffer_manager_->get();
        std::vector<cv::Mat> tmpVec;
        for (std::pair<int, cv::Mat> pair : tmpMap) {
          tmpVec.push_back(pair.second);
        }
        update_frames(tmpVec);
        make_luminance_balance();
        stitch_all_parts();
        make_white_balance();
        copy_car_image();
        sptr_buffer_->add(image_.clone(), drop_if_full_);
      }
      update_fps(processing_time_);
      stat_data_.frames_processed_count_ += 1;
      //update_statistics_gui.emit(self.stat_data)
    }
  }
  protected:
  cv::Mat FL() const {
    return image_(cv::Range(0,static_settings.yt),cv::Range(0,static_settings.xl));
  }
  cv::Mat F() const {
    return image_(cv::Range(0,static_settings.yt),cv::Range(static_settings.xl,static_settings.xr));
  }
  cv::Mat FR() const {
    return image_(cv::Range(0,static_settings.yt),cv::Range(static_settings.xr,image_.cols));
  }
  cv::Mat BL() const {
    return image_(cv::Range(static_settings.yb,image_.rows),cv::Range(0,static_settings.xl));
  }
  cv::Mat B() const {
    return image_(cv::Range(static_settings.yb,image_.rows),cv::Range(static_settings.xl,static_settings.xr));
  }
  cv::Mat BR() const {
    return image_(cv::Range(static_settings.yb,image_.rows),cv::Range(static_settings.xr,image_.cols));
  }
  cv::Mat L() const {
    return image_(cv::Range(static_settings.yt,static_settings.yb),cv::Range(0,static_settings.xl));
  }
  cv::Mat R() const {
    return image_(cv::Range(static_settings.yt,static_settings.yb),cv::Range(static_settings.xr,image_.cols));
  }
  cv::Mat C() const {
    return image_(cv::Range(static_settings.yt,static_settings.yb),cv::Range(static_settings.xl,static_settings.xr));
  }
  bool drop_if_full_ = false;
  std::shared_ptr<Buffer<cv::Mat>> sptr_buffer_ = std::shared_ptr<Buffer<cv::Mat>>(nullptr);
  std::shared_ptr<ProjectedImageBufferManager> sptr_proc_buffer_manager_ = std::shared_ptr<ProjectedImageBufferManager>(nullptr);
  cv::Mat car_image_ = static_settings.car_image.clone();
  cv::Mat image_ = cv::Mat::zeros(static_settings.total_h, static_settings.total_w,CV_8UC3);
  std::vector<cv::Mat> vec_images_, vec_weights_, vec_masks_; //dtype CV_8U, CV_64F, CV_8U
  cv::Mat F_, B_, L_, R_, FL_, FR_, BL_, BR_, C_;
};
} //namespace SVS

#endif //SVS_BIRDVIEW_HPP