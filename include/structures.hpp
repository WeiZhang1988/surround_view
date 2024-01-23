#ifndef SVS_STRUCTURES_HPP
#define SVS_STRUCTURES_HPP

#include <opencv2/opencv.hpp>

namespace SVS{
class ImageFrame{
  public:
  ImageFrame() {}
  ImageFrame(double _timestamp, cv::Mat _image) : timestamp_(_timestamp) {
    image_ = _image.clone();
  }
  double timestamp_;
  cv::Mat image_;
};

class ThreadStatisticsData{
  public:
  double average_fps_;
  int frames_processed_count_;
};

} //namespace SVS

#endif //SVS_STRUCTURES_HPP