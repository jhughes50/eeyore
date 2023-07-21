#include "eeyore/boson.hpp"

int main()
{
  // set the serial port and baud rate	
  int32_t serial_dev = 47;
  int32_t serial_baud = 921600;

  // instantiate the class
  Boson boson(serial_dev, serial_baud, 640, 512, "/dev/boson_video", "boson");

  //define where the calibration .yaml file is
  std::string cal_path;
  cal_path = "/home/jbeason/catkin_ws/src/eeyore/cal/ir_calibration.yaml";

  // get and set the intrinsic and distance coeffs from the calibration file
  cv::Mat intrinsic = boson.getParams(cal_path, "K");
  cv::Mat distance = boson.getParams(cal_path, "D");

  boson.setIntrinsicCoeffs(intrinsic);
  boson.setDistanceCoeffs(distance);

  // conduct flat field calibration (FCC) and open up the data link to the sensor
  int result = boson.conductFcc();
  result = boson.openSensor();

  cv::Mat img;

  // loop
  while (true)
  {
    img= boson.getFrame();
    cv::imshow("boson_test",img);
    cv::waitKey(1);
  }

  boson.closeSensor();

  return 0;
}
