## Eeyore ##

A repo for EO/IR Camera payloads using the Sinnaker SDK for the EO camera (usually a FLIR blackfly) and a FLIR Boson as the IR camera.

### Boson IR Camera ###

When instatiating the boson class the inputs are as follows
1. `serial_dev (int32)`: this the numerical index of where the camera is supposed to be mounted,  `47 -> /dev/boson_ser` 
2. `serial_baud (int32)`: baudrate of the camera, default is 921600
3. `width (int)`: width of the image, default value to use is 640
4. `height (int)`: height of the image, default vale to use is 512
5. `video_id (string)`: where the video stream of the camera is mounted
6. `sensor_name (string)`: unique ID for the sensor

To get pictures in a loop from the Boson follow the code below:
```cpp
#include <eeyore/boson.hpp>

int main()
{
  // set the serial port and baud rate	
  int32_t serial_dev = 47;
  int32_t serial_baud = 921600;

  // instantiate the class 
  Boson boson(serial_dev, serial_baud, 640, 512, "/dev/boson_video", "boson");

  //define where the calibration .yaml file is
  std::string cal_path;
  cal_path = "/home/user1/bhg2_ws/src/eeyore/cal/ir_calibration.yaml";

  // get and set the intrinsic and distance coeffs from the calibration file
  cv::Mat intrinsic = boson.getParams(cal_path, "K");
  cv::Mat distance = boson.getParams(cal_path, "D");

  boson.setIntrinsicCoeffs( intrinsic );
  boson.setDistanceCoeffs( distance );

  // conduct flat field calibration (FCC) and open up the data link to the sensor
  result = boson.conductFcc();
  result = boson.openSensor();

  cv::Mat img;

  // loop
  while (true)
  {
    img= boson.getFrame();
  }

  boson.closeSensor();

  return 0;
}
```

### EO Camera ###
The EO module in Eeyore should be able to get pictures and configure any camera that is capable of talking with the spinnaker SDK. You need Spinnaker3.0.0.118 (or later), although this has only been tested on 3.0.0.118. Any version earlier than this will not work!
The values for instantiating the class are as follows:
1. `height (int)`: image height, use zero for max
2. `width (int)`: iamge width, use zer for max
3. `trigger (TriggerType)`: A value from the TriggerType enumeration to define what trigger to set

The `TriggerType` Enumeration has the following options:
- SOFTWARE: uses a software trigger
- HARDWARE_LINE0: hardware trigger where the pulse is coming in on port 0 of the connector
- HARDWARE_LINE{1,2,3}: hardware trigger where the pulse is coming in on port {1,2,3} of the connector

The camera can be used like the following code:
```cpp
#include "eeyore/electro_optical.hpp"

int main()
{
  // define the trigger type
  TriggerType trig = HARDWARE_LINE3;

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
  std::string cal_path = "/home/user1/catkin_ws/src/eeyore/cal/eo_calibration.yaml";

  // get and set the intrinsic and distance parameters
  cv::Mat intrinsic = blackfly.getParams(cal_path, "K");
  cv::Mat distance = blackfly.getParams(cal_path, "D");

  blackfly.setIntrinsicCoeffs( intrinsic );
  blackfly.setDistanceCoeffs( distance );

  //optional: get camera serial number and print the device info
  std::string ser_num = blackfly.getSerialNumberFromCam();
  blackfly.printDeviceInfo();

  cv::Mat img;
  int result;

  while (true)
  {
    img = blackfly.getFrame();
    // OR write a the image directly (i.e. no conversion to opencv is done)
    // result = blackfly.writeFrame("frame.png");
   }

  blackfly.closeDevice();

  return 0;
}