#include <iostream>
#include <opencv2/calib3d.hpp>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

int main(int argc, char **argv) {
  (void)argc;
  (void)argv;
  // 1, find corners in chessboard
  std::vector<cv::String> fileNames;
  cv::glob("./data/images/template_calibrate_camera/Image*.png", fileNames, false);
  // The board has 25 x 18 fields with a size of 15x15mm
  cv::Size patternSize(24, 17);
  std::vector<std::vector<cv::Point2f>> imgpoints;
  std::vector<std::vector<cv::Point3f>> objpoints;
  std::vector<cv::Point3f> objp;
  for(int i = 0; i<patternSize.height; i++){
    for(int j = 0; j<patternSize.width; j++){
      objp.push_back(cv::Point3f(j,i,0));
    }
  }
  int i = 0;
  for (auto const &f : fileNames) {
    std::cout << "searching for chessboard corners in image file " + std::string(f) + "..."<< std::endl;
    cv::Mat img = cv::imread(fileNames[i]);
    cv::Mat gray;
    cv::cvtColor(img, gray, cv::COLOR_RGB2GRAY);
    std::vector<cv::Point2f> corners;
    bool patternFound = cv::findChessboardCorners(gray, patternSize, corners, cv::CALIB_CB_ADAPTIVE_THRESH + cv::CALIB_CB_NORMALIZE_IMAGE + cv::CALIB_CB_FAST_CHECK);
    // 2. Use cv::cornerSubPix() to refine the found corner detections
    if(patternFound){
        cv::cornerSubPix(gray, corners,cv::Size(11,11), cv::Size(-1,-1), cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::MAX_ITER, 30, 0.1));
        std::cout<<"OK."<<std::endl;
        imgpoints.push_back(corners);
        objpoints.push_back(objp);
        cv::drawChessboardCorners(img, patternSize, corners, patternFound);
    }
    cv::imshow("chessboard detection", img);
    cv::waitKey(0);
    i++;
  }
  // 2, calibrate
  // intrinsic camera matrix
  cv::Matx33d K(cv::Matx33d::eye()); 
  // distortion coefficients 
  cv::Vec<double, 5> d(0, 0, 0, 0, 0); 
  std::vector<cv::Mat> rvecs, tvecs;
  std::vector<double> stdIntrinsics, stdExtrinsics, perViewErrors;
  int flags = cv::CALIB_FIX_ASPECT_RATIO + cv::CALIB_FIX_K3 + cv::CALIB_ZERO_TANGENT_DIST + cv::CALIB_FIX_PRINCIPAL_POINT;
  cv::Size imageSize(1440, 1080);
  std::cout << "Calibrating..." << std::endl;
  float error = cv::calibrateCamera(objpoints, imgpoints, imageSize, K, d, rvecs, tvecs, flags);
  std::cout << "Reprojection error = " << error << std::endl;
  std::cout << "intrinsic camera matrix = " << K << std::endl;
  std::cout << "distortion coefficients = " << d << std::endl;
  // 3, precompute lens correction interpolation
  // alpha trims images
  const double alpha = 0.;
  cv::Mat New_K = getOptimalNewCameraMatrix(K, d, imageSize, alpha, imageSize, 0);
  cv::Mat mapX, mapY;
  cv::initUndistortRectifyMap(K, d, cv::Matx33d::eye(), New_K, imageSize, CV_32FC1,mapX, mapY);
  // 4, how lens corrected images
  for (auto const &f : fileNames) {
    std::cout << std::string(f) << std::endl;
    cv::Mat img = cv::imread(f, cv::IMREAD_COLOR);
    cv::Mat imgUndistorted;
    cv::remap(img, imgUndistorted, mapX, mapY, cv::INTER_LINEAR);
    cv::imshow("undistorted image", imgUndistorted);
    cv::waitKey(0);
  }
  return 0;
}