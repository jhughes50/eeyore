#include "eeyore/electro_optical.hpp"

int main()
{
  // define the trigger type
  std::string trig =  "HARDWARE_LINE3";

  // instantiate the class
  ElectroOpticalCam blackfly(0,0,trig);
  // setup the camera
  // tell it what trigger to look for
  blackfly.configureTrigger();
  // set parameters
  blackfly.setupCamera();
  // start the image stream
  blackfly.startCamera();

  // define the path to the .yaml file containing the calibration parameters
  std::string cal_path = "/home/jbeason/catkin_ws/src/eeyore/cal/eo_calibration.yaml";

  // get and set the intrinsic and distance parameters
  cv::Mat intrinsic = blackfly.getParams(cal_path, "K");
  cv::Mat distance = blackfly.getParams(cal_path, "D");

  blackfly.setIntrinsicCoeffs( intrinsic );
  blackfly.setDistanceCoeffs( distance );

  //optional: get camera serial number and print the device info
  std::string ser_num = blackfly.getSerialNumberFromCam();
  blackfly.printDeviceInfo();

  cv::Mat img;
  cv::Mat resized_img;
  cv::Size2d new_size;
  new_size.width = 1024;
  new_size.height = 750;

  while (true)
  {
    img = blackfly.getFrame();
    cv::resize(img,resized_img,new_size,0,0,cv::INTER_LANCZOS4);
    cv::imshow("electro_optical_test",resized_img);
    cv::waitKey(1);
    // OR write a the image directly (i.e. no conversion to opencv is done)
    // result = blackfly.writeFrame("frame.png");
  }

  blackfly.closeDevice();

  return 0;
}


