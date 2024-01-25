#ifndef SVS_UTILS_HPP
#define SVS_UTILS_HPP

#include <stdexcept>
#include <opencv2/opencv.hpp>

namespace SVS{
class utils{
  public:
  cv::Mat convert_binary_to_bool(cv::Mat _mask) {
    cv::Mat tmp;
    _mask.convertTo(tmp, CV_64F);
    cv::Mat result = _mask / 255.0;
    result.convertTo(result, CV_8U);
    return result.clone();
  }
  double get_mean_statistics(cv::Mat _gary, cv::Mat _mask) {
    return double(cv::sum(_gary.clone().mul(_mask.clone()))[0]);
  }
  double mean_luminance_ratio(cv::Mat _grayA, cv::Mat _grayB, cv::Mat _mask) {
    return double(get_mean_statistics(_grayA, _mask)) / double(get_mean_statistics(_grayB, _mask));
  }
  cv::Mat adjust_luminance(cv::Mat _gray, double _factor) {
    cv::Mat tmp;
    _gray.convertTo(tmp, CV_64F);
    cv::Mat result;
    result = cv::min(tmp * _factor, 255.0);
    result.convertTo(result, CV_8U);
    return result;
  }
  cv::Mat get_mask(cv::Mat _img) {
    cv::Mat gray;
    cv::cvtColor(_img,gray,cv::COLOR_BGR2GRAY);
    cv::Mat mask;
    double ret = cv::threshold(gray,mask,0,255,cv::THRESH_BINARY);
    return mask.clone();
  }
  cv::Mat get_overlap_region_mask(cv::Mat _imA, cv::Mat _imB) {
    cv::Mat overlap;
    cv::bitwise_and(_imA.clone(),_imB.clone(),overlap);
    cv::Mat mask = get_mask(overlap);
    cv::dilate(mask.clone(),mask,cv::Mat::ones(2,2,CV_8U),cv::Point(-1,-1),2);
    return mask.clone();
  }
  cv::Mat get_outmost_polygon_boundary(cv::Mat _img) {
    cv::Mat mask = get_mask(_img.clone());
    cv::dilate(mask.clone(),mask,cv::Mat::ones(2,2,CV_8U),cv::Point(-1,-1),2);
    std::vector<std::vector<cv::Point> > contours;
    cv::findContours(mask,contours,cv::RETR_EXTERNAL,cv::CHAIN_APPROX_SIMPLE);
    std::sort(contours.begin(), contours.end(), [](std::vector<cv::Point> contour1, std::vector<cv::Point> contour2){
      return fabs( cv::contourArea(cv::Mat(contour1)) ) > fabs( cv::contourArea(cv::Mat(contour2)) );
    });
    cv::Mat polygon;
    cv::approxPolyDP(cv::Mat(contours[0]),polygon,0.009 * cv::arcLength(cv::Mat(contours[0]),true),true);
    return polygon.clone();
  }
  void get_weight_mask_matrix(cv::Mat _imA, cv::Mat _imB, cv::Mat &_out_G, cv::Mat &_out_overlapMask, double _dist_threshold = 5.0) {
    _out_overlapMask = get_overlap_region_mask(_imA.clone(),_imB.clone());
    cv::Mat overlapMaskInv, imA_diff, imB_diff, polyA, polyB;
    cv::bitwise_not(_out_overlapMask.clone(),overlapMaskInv);
    cv::bitwise_and(_imA.clone(),_imA.clone(),imA_diff,overlapMaskInv);
    cv::bitwise_and(_imB.clone(),_imB.clone(),imB_diff,overlapMaskInv);
    polyA = get_outmost_polygon_boundary(imA_diff.clone());
    polyB = get_outmost_polygon_boundary(imB_diff.clone());
    get_mask(_imA.clone()).convertTo(_out_G, CV_64F);
    _out_G /= 255.0;
    std::vector<cv::Point> indices; 
    cv::findNonZero(_out_overlapMask==255, indices);
    for (cv::Point point : indices) {
      double distToB = cv::pointPolygonTest(polyB, point, true);
      if (distToB < _dist_threshold) {
        double distToA = cv::pointPolygonTest(polyA, point, true);
        distToB *= distToB;
        distToA *= distToA;
        _out_G.at<double>(point.y,point.x) = distToB / (distToA + distToB);
      }
    }
  }
  cv::Mat make_white_balance(cv::Mat _image) {
    std::vector<cv::Mat> channels;
    cv::split(_image.clone(), channels);
    cv::Mat B = channels[0].clone();
    cv::Mat G = channels[1].clone();
    cv::Mat R = channels[2].clone();
    double m1 = cv::mean(B)[0];
    double m2 = cv::mean(G)[0];
    double m3 = cv::mean(R)[0];
    double k  = (m1 + m2 + m3) / 3;
    double c1 = k / m1;
    double c2 = k / m2;
    double c3 = k / m3;
    B = adjust_luminance(B, c1);
    G = adjust_luminance(G, c2);
    R = adjust_luminance(R, c3);
    cv::Mat result;
    cv::merge(std::vector<cv::Mat>{B,G,R},result);
    return result.clone();
  }
};
static utils static_utils = utils();
} //namespace SVS
#endif //SVS_UTILS_HPP