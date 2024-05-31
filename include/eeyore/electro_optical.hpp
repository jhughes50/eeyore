/* Author: Jason Hughes
 * Date: April 2023
 * About: header file
 */ 

#ifndef ELECTRO_OPTICAL_CAM
#define ELECTRO_OPTICAL_CAM

#include "Spinnaker.h"
#include "SpinGenApi/SpinnakerGenApi.h"
#include <opencv2/opencv.hpp>
#include <sstream>
#include <string>

using namespace Spinnaker;
using namespace Spinnaker::GenApi;
using namespace Spinnaker::GenICam;

enum TriggerType
  {
    SOFTWARE,
    HARDWARE_LINE0,
    HARDWARE_LINE1,
    HARDWARE_LINE2,
    HARDWARE_LINE3
  };

  
class ElectroOpticalCam
{
public:
  //constructor
  ElectroOpticalCam( int h, int w, std::string t );
  ElectroOpticalCam() = default;

  //setters
  void setHeight( int h );
  void setWidth( int w );
  void setTrigger( TriggerType t );
  void setIntrinsicCoeffs( cv::Mat int_coeffs );
  void setDistanceCoeffs( cv::Mat dist_coeffs );
  
  //getters
  int getHeight();
  int getWidth();
  TriggerType getTrigger();
  cv::Mat getIntrinsicCoeffs();
  cv::Mat getDistanceCoeffs();
  
  //functions
  int configureTrigger();
  int resetTrigger();
  void initCam();
  int setupCamera();
  int startCamera();
  cv::Mat getFrame();
  int writeFrame(std::string filename);
  cv::Mat getParams(std::string file_path, std::string data);
  void closeDevice();
  void quickStart();
  void printDeviceInfo();
  std::string getSerialNumberFromCam();

  
private:

  int height_;
  int width_;
  bool rectify_;

  SystemPtr system_;
  CameraPtr cam_;
  CameraList cam_list_;
  
  ImageProcessor processor_;

  TriggerType trig_;

  cv::Mat intrinsic_coeffs_;
  cv::Mat distance_coeffs_;

  std::string serial_number_;
};
#endif
  
