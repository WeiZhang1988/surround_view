#ifndef SVS_STRUCTURES_HPP
#define SVS_STRUCTURES_HPP

#include <opencv2/opencv.hpp>

namespace SVS{
class ImageFrame{
  public:
  ImageFrame(double _timestamp, cv::Mat _image) : timestamp_(_timestamp), image_(_image) {}
  protected:
  double timestamp_;
  cv::Mat image_;
};

class ThreadStatisticsData{
  protected:
  double average_fps_;
  int frames_processed_count_;
};

} //namespace SVS

#endif //SVS_STRUCTURES_HPP