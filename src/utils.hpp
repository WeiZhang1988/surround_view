#ifndef SVS_UTILS_HPP
#define SVS_UTILS_HPP

#include <opencv2/opencv.hpp>

namespace SVS{
cv::Mat convert_binary_to_bool(cv::Mat _mask) {
  cv::Mat result = _mask / 255.0;
  result.convertTo(result, cv::CV_8U);
  return result.clone();
}
} //namespace SVS
#endif //SVS_UTILS_HPP