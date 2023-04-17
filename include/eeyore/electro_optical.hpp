/* Author: Jason Hughes
 * Date: April 2023
 * About: header file
 */ 

#ifndef ELECTRO_OPTICAL_CAM
#define ELECTRO_OPTICAL_CAM

#include "Spinnaker.h"
#include "SpinGenApi/SpinnakerGenApi.h"

using namespace Spinnaker;
using namespace Spinnaker::GenApi;
using namespace Spinnaker::GenICam;

enum TriggerType
  {
    SOFTWARE,
    HARDWARE
  }

  
class ElectroOpticalCam
{
public:
  //constructor
  ElectroOpticalCam();

  //setters
  void setHeight( int h );
  void setWidth( int w );

  //getters
  int getHeight();
  int getWidth();

  //functions
  int configureTrigger( TriggerType trig );
  int resetTrigger();
  int acquireImage();
  
  
private:

  int height_;
  int width_;

  SystemPtr system_;
  CameraPtr cam_;


};
#endif
  
