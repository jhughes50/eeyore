/* Author: Jason Hughes
 * Date: April 2023
 * About: Header file for interfacing with boson camera
 */

#ifndef BOSON_HPP
#define BOSON_HPP

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <cv_bridge/cv_bridge.h>
#include <image_transport/image_transport.h>
#include <string>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>    
#include <sys/ioctl.h> 
#include <asm/types.h> 
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/videodev2.h>

#include "ros/ros.h"

extern "C"
{
#include "boson/EnumTypes.h"
#include "boson/UART_Connector.h"
#include "boson/Client_API.h"
}

#define CLEAR(x) memset(&(x), 0, sizeof(x))

using namespace cv;

class Boson
{
public:
  // constructor
  Boson( int32_t serial_dev, int32_t serial_baud, int width, int height, std::string video_id, std::string sensor_name );
  // destructor
  ~Boson();

  // setters
  void setSerialDev( int32_t serial_dev );
  void setSerialBaud( int32_t serial_baud );
  void setWidth( int w );
  void setHeight( int h );
  void setVideoId( std::string video_id );
  void setSensorName( std::string name );
  void setIntrinsicCoeffs( cv::Mat int_coeffs );
  void setDistanceCoeffs( cv::Mat dist_coeffs );
  
  // getters
  int32_t getSerialDev();
  int32_t getSerialBaud();
  int getWidth();
  int getHeight();
  std::string getVideoId();
  std::string getSensorName();
  cv::Mat getIntrinsicCoeffs();
  cv::Mat getDistanceCoeffs();
  
  // others
  int openSensor();
  int closeSensor();
  cv::Mat getFrame();
  void grayScale16( Mat input_16, Mat output_16, int height, int width );
  void AgcBasicLinear( Mat input_16, Mat output_16, int height, int width );
  int conductFcc();
  int printCamInfo();
  std::string getSerialNumber();
  cv::Mat getParams(std::string file_path, std::string data);
  
private:
  // class variables
  int32_t serial_dev_;
  int32_t serial_baud_;
  int width_;
  int height_;
  std::string video_id_;
  std::string sensor_name_;
  std::string serial_number_;
  
  int fd_;
  struct v4l2_format format_;
  struct v4l2_buffer bufferinfo_;
  
  Mat thermal16_;
  Mat thermal16_linear_;
  Mat thermal16_out_;

  Mat intrinsic_coeffs_;
  Mat distance_coeffs_;
};
#endif
  
  
  
  
