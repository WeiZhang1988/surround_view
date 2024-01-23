#ifndef SVS_SIMPLE_GUI_HPP
#define SVS_SIMPLE_GUI_HPP

#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <opencv2/opencv.hpp>
#include "utils.hpp"

namespace SVS{
int display_image(std::string _window_title, cv::Mat _image) {
  cv::imshow(_window_title,_image);
  while (true) {
    double click = cv::getWindowProperty(_window_title, cv::WND_PROP_AUTOSIZE);
    if (click < 0) {
      return -1;
    } 
    int key = (cv::waitKey(1) & 0xFF);
    if (key == int('q')) {
      return -1;
    }
    if (key == 13) {
      return 1;
    }
  }
}
class PointSelector{
  public:
  PointSelector(cv::Mat _image, std::string _title = "PointSelector") 
    : image_(_image), title_(_title){
      keypoints_ = std::vector<cv::Point>{};
  }
  void draw_image() {
    cv::Mat new_image = image_.clone();
    for (int i=0; i<keypoints_.size(); i++) {
      cv::circle(new_image, keypoints_[i], 6, POINT_COLOR_, -1);
      cv::putText(new_image, std::to_string(i), cv::Point(keypoints_[i].x, keypoints_[i].y - 15), cv::FONT_HERSHEY_SIMPLEX, 0.6, POINT_COLOR_, 2);
    }
    if (keypoints_.size() == 2) {
      cv::line(new_image, keypoints_[0], keypoints_[1], POINT_COLOR_, 2);
    } else if (keypoints_.size() > 2) {
      cv::Mat mask = create_mask_from_pixels(keypoints_, image_.size());
      draw_mask_on_image(new_image, mask);
    }
    cv::imshow(title_,new_image);
  }
  void onclick_internal(int _event, int _x, int _y) {
    if (_event == cv::EVENT_LBUTTONDOWN) {
      std::cout<<"click "<<_x<<", "<<_y<<std::endl;
      keypoints_.push_back(cv::Point(_x,_y));
      draw_image();
    }
  }
  static void onclick(int _event, int _x, int _y, int flag, void *obj) {
    PointSelector *ptr_app = reinterpret_cast<PointSelector *>(obj);
    if (ptr_app) {
      ptr_app->onclick_internal(_event,_x,_y);
    }
  }
  friend void onclick(cv::MouseEventTypes _event, int _x, int _y, cv::MouseEventFlags flat, void *obj);
  bool loop() {
    cv::namedWindow(title_);
    cv::setMouseCallback(title_,onclick,this);
    cv::imshow(title_,image_);
    while (true) {
      double click = cv::getWindowProperty(title_, cv::WND_PROP_AUTOSIZE);
      if (click < 0) {
        return false;
      }
      int key = (cv::waitKey(1) & 0xFF);
      if (key == int('q')) {
        return -1;
      }
      if (key == int('d')) {
        if (keypoints_.size() > 0) {
          cv::Point point = keypoints_.back();
          keypoints_.pop_back();
          std::cout<<"delete (" + std::to_string(point.x) + ", " + std::to_string(point.y)  + ")"<<std::endl;
          draw_image();
        }
      }
      if (key == 13) {
        return true;
      }
    }
  }
  cv::Mat create_mask_from_pixels(std::vector<cv::Point> _pixels,cv::Size _image_size) {
    std::vector<cv::Point> hull;
    cv::convexHull(_pixels,hull);
    cv::Mat mask = cv::Mat::zeros(_image_size,CV_8U);
    cv::fillConvexPoly(mask,hull,cv::Scalar(1),8,0);
    mask = static_utils.convert_binary_to_bool(mask);
    return mask.clone();
  }
  cv::Mat draw_mask_on_image(cv::Mat _image, cv::Mat _mask) {
    cv::Mat new_image = cv::Mat::zeros(_image.cols,_image.rows,_image.channels());
    new_image(cv::Range::all(),cv::Range::all()) = FILL_COLOR_;
    cv::Mat new_mask;
    cv::bitwise_and(new_image,new_image,new_mask,_mask);
    cv::addWeighted(_image, 1.0, new_mask, 0.5, 0.0, _image);
    return _image.clone();
  }
  protected:
  cv::Scalar POINT_COLOR_{0,0,255};
  cv::Scalar FILL_COLOR_{0,255,255};
  cv::Mat image_;
  std::string title_;
  std::vector<cv::Point> keypoints_;
};
}//namespace SVS
#endif //SVS_SIMPLE_GUI_HPP