/* Author: Jason Hughes
 * Date: April 2023
 * About: header file
 */ 

#ifndef ELECTRO_OPTICAL_CAM
#define ELECTRO_OPTICAL_CAM

#include "Spinnaker.h"
#include "SpinGenApi/SpinnakerGenApi.h"
#include <opencv2/opencv.hpp>

using namespace Spinnaker;
using namespace Spinnaker::GenApi;
using namespace Spinnaker::GenICam;

enum TriggerType
  {
    SOFTWARE,
    HARDWARE
  };

  
class ElectroOpticalCam
{
public:
  //constructor
  ElectroOpticalCam( int h, int w );

  //setters
  void setHeight( int h );
  void setWidth( int w );
  void setTrigger( TriggerType t );

  //getters
  int getHeight();
  int getWidth();
  TriggerType getTrigger();

  //functions
  int configureTrigger();
  int resetTrigger();
  int setupCamera();
  int startCamera();
  cv::Mat acquireImage();
  
  
private:

  int height_;
  int width_;

  SystemPtr system_;
  CameraPtr cam_;

  ImageProcessor processor_;

  TriggerType trig_;
};
#endif
  
