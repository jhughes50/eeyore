/* Author: Jason Hughes
 * Date: April 2023
 * About: Class for Boson Camera Object
 */

#include "eeyore/boson.hpp"

Boson::Boson( int32_t serial_dev, int32_t serial_baud, int width, int height, std::string video_id, std::string sensor_name )
{
  setSerialDev( serial_dev );
  setSerialBaud( serial_baud );
  setWidth( width );
  setHeight( height );
  setVideoId( video_id );
  setSensorName( sensor_name );
  rectify_ = false;
}

Boson::~Boson()
{
  exit(1);
}

void Boson::setSerialDev( int32_t serial_dev )
{
  serial_dev_ = serial_dev;
}

void Boson::setSerialBaud( int32_t serial_baud )
{
  serial_baud_ = serial_baud;
}

void Boson::setWidth( int w )
{
  width_ = w;
}

void Boson::setHeight( int h )
{
  height_ = h;
}

void Boson::setVideoId( std::string video_id )
{
  video_id_ = video_id;
}

void Boson::setSensorName( std::string name )
{
  sensor_name_ = name;
}

void Boson::setIntrinsicCoeffs( cv::Mat int_coeffs )
{
  intrinsic_coeffs_ = int_coeffs;
}

void Boson::setDistanceCoeffs( cv::Mat dist_coeffs )
{
  distance_coeffs_ = dist_coeffs;
}

int32_t Boson::getSerialDev()
{
  return serial_dev_;
}

int32_t Boson::getSerialBaud()
{
  return serial_baud_;
}

int Boson::getWidth()
{
  return width_;
}

int Boson::getHeight()
{
  return height_;
}

std::string Boson::getVideoId()
{
  return video_id_;
}

std::string Boson::getSensorName()
{
  return sensor_name_;
}

cv::Mat Boson::getIntrinsicCoeffs()
{
  return intrinsic_coeffs_;
}

cv::Mat Boson::getDistanceCoeffs()
{
  return distance_coeffs_;
}

int Boson::openSensor()
{
  struct v4l2_capability cap;
  std::cout << "[BOSON] Attempting to connect to camera" << std::endl;

  if ((fd_ = open(video_id_.c_str(), O_RDWR)) < 0 )
    { 
      perror("[BOSON] ERROR: Invalid video device");
      exit(1);
    }
  if (ioctl(fd_, VIDIOC_QUERYCAP, &cap) < 0)
    {
      perror("[BOSON] ERROR: VIDIOC_QUERYCAP Video Capture is not avaialable");
      exit(1);
    }

  if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
    {
      perror("[BOSON] ERROR: this device does not handle single-planar video capture");
      exit(1);
    }
  
  CLEAR(format_);

  format_.fmt.pix.pixelformat = V4L2_PIX_FMT_Y16;

  format_.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  format_.fmt.pix.width = width_;
  format_.fmt.pix.height = height_;

  if (ioctl(fd_, VIDIOC_S_FMT, &format_) < 0)
    {
      perror("[BOSON] ERROR: VIDIO_S_FMT");
      exit(1);
    }
  
  struct v4l2_requestbuffers bufrequest;
  bufrequest.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  bufrequest.memory = V4L2_MEMORY_MMAP;
  bufrequest.count = 1; 

  if (ioctl(fd_, VIDIOC_REQBUFS, &bufrequest) < 0)
    {
      perror("[BOSON] ERROR: VIDIO_REQBUFS");
      exit(1);
    }

  memset(&bufferinfo_, 0, sizeof(bufferinfo_));

  bufferinfo_.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  bufferinfo_.memory = V4L2_MEMORY_MMAP;
  bufferinfo_.index = 0;
  
  if (ioctl(fd_, VIDIOC_QUERYBUF, &bufferinfo_) < 0)
    {
      perror("[BOSON] ERROR: VIDIO_QUERTBUF");
      exit(1);
    }

  void *buffer_start = mmap(NULL, bufferinfo_.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, bufferinfo_.m.offset);

  if (buffer_start == MAP_FAILED)
    {
      perror("[BOSON] ERROR: mmap failed");
      exit(1);
    }

  memset(buffer_start, 0, bufferinfo_.length);

  int type = bufferinfo_.type;

  if (ioctl(fd_, VIDIOC_STREAMON, &type) < 0)
    {
      perror("[BOSON] ERROR: VIDIOC_STREMON");
      exit(1);
    }

  
  thermal16_ = cv::Mat(height_, width_, CV_16UC1, buffer_start);
  thermal16_linear_ = cv::Mat(height_, width_, CV_8UC1, 1);
  thermal16_out_ = cv::Mat(height_, width_, CV_16UC1, 1);

  std::cout << "[BOSON] Successfully conected to camera" << std::endl;
  
  return 1;
}
  
int Boson::closeSensor()
{
  int type = bufferinfo_.type;

  if (ioctl(fd_, VIDIOC_STREAMOFF, &type) < 0)
    {
      perror("[BOSON] ERROR: VIDIOC_STREAMOFF");
      exit(1);
    }

  close(fd_);

  std::cout << "[BOSON] Exited cleanly" << std::endl;

  return EXIT_SUCCESS;
}

cv::Mat Boson::getFrame()
{
  // Put the buffer in the incoming queue.
  if (ioctl(fd_, VIDIOC_QBUF, &bufferinfo_) < 0)
    {
      perror("[BOSON] ERROR: VIDIOC_QBUF");
      exit(1);
    }

  // The buffer's waiting in the outgoing queue.
  if (ioctl(fd_, VIDIOC_DQBUF, &bufferinfo_) < 0)
    {
      perror("[BOSON] ERROR: VIDIOC_QBUF");
      exit(1);
    } 

  grayScale16(thermal16_, thermal16_out_, height_, width_);

  cv::Mat thermal16_final;
  if (rectify_ == true)
    {
      cv::undistort(thermal16_out_, thermal16_final, intrinsic_coeffs_, distance_coeffs_);
    }
  else
    {
      thermal16_final = thermal16_out_;
    }
  return thermal16_final;
}

void Boson::grayScale16(Mat input_16, Mat output_16, int height, int width)
{
  // auxiliary variables for AGC calcultion
  unsigned int max1 = 0;      // 16 bits
  unsigned int min1 = 65535;      // 16 bits
  unsigned int value1, value2;
  size_t sizeInBytes = input_16.step[0] * input_16.rows;

  for (int i = 0; i < height; i++)
    {
      for (int j = 0; j < width; j++)
	{
	  value1 = input_16.at<uint16_t>(i, j);
	  if (value1 <= min1)
	    {
	      min1 = value1;
	    }
	  if (value1 >= max1)
	    {
	      max1 = value1;
	    }
	}
    }

  for (int i = 0; i < height; i++)
    {
      for (int j = 0; j < width; j++)
	{
	  value1 = input_16.at<uint16_t>(i, j);
	  value2 = ((65535 * (value1 - min1))) / (max1 - min1);
	  output_16.at<uint16_t>(i, j) = value2;
	}
    }
}

int Boson::conductFcc()
{

  std::cout << "[BOSON] Conducting flat field calibration" << std::endl;

  FLR_RESULT result;
  result = Initialize(serial_dev_, serial_baud_);

  if (result)
    {
      perror("[BOSON] Failed to initialize FFC");
      Close();
      return -1;
    }
  else
    {
      std::cout << "[BOSON] Intialized FFC successfully" << std::endl;
    }

  result = bosonRunFFC();

  if (result)
    {
      perror("[BOSON] Failed to run FFC");
      return -1;
    }
  else
    {
      std::cout << "[BOSON] Successfully ran FFC" << std::endl;
    }
  sleep(3);
  Close();

  return 0;
}

int Boson::printCamInfo()
{
  FLR_RESULT result;

  result = Initialize(serial_dev_, serial_baud_);

  if (result)
    {
      std::cerr << "[BOSON] Failed to get camera info, aborting" << std::endl;
      Close();
      return -1;
    }

  uint32_t serial_num;

  result = bosonGetCameraSN(&serial_num);

  if (result)
    {
      perror("[BOSON] Failed to get camera serial number, aborting");
      Close();
      return -1;
    }
  else
    {
      serial_number_ = std::to_string(serial_num);
      std::cout << "[BOSON] Talking to camera with serial number: " << serial_number_ << std::endl;
    }

  uint32_t major, minor, patch;
  result = bosonGetSoftwareRev(&major, &minor, &patch);

  if (result)
    {
      std::cerr << "[BOSON] Failed to get camera software info, aborting" << std::endl;
      Close();
      return -1;
    }
  else
    {
      std::cout << "[BOSON] Software: " << major << ", " << minor << ", " << patch << std::endl;
    }

  FLR_BOSON_SENSOR_PARTNUMBER_T part_num;
  result = bosonGetSensorPN(&part_num);

  if (result)
    {
      perror("[BOSON] Failed to get part number info, aborting");
      Close();
      return -1;
    }
  else
    {
      std::cout << "[BOSON] Part number: " << part_num.value << std::endl;
    }

  FLR_BOSON_EXT_SYNC_MODE_E sync_mode;
  result = bosonGetExtSyncMode(&sync_mode);

  if(result)
    {
      std::cout << "[BOSON] Failed to get sync mode info, aborting" << std::endl;
      Close();
      return -1;
    }
  else
    {
      std::string sync_mode_str = "disabled";
      if (sync_mode == 1)
	{
	  sync_mode_str = "Manager";
	}
      else if (sync_mode == 2)
	{
	  sync_mode_str = "Worker";
	}

      std::cout << "[BOSON] Camera sync mode: " << sync_mode_str.c_str() << std::endl;
    }

  Close();
  return 0;
}

std::string Boson::getSerialNumber()
{
  FLR_RESULT result;
  result = Initialize(serial_dev_, serial_baud_);

  if (result)
    {
      std::cerr << "[BOSON] Failed to get camera serial number, cant connect to camera, aborting" << std::endl;
      Close();
      exit(-1);
    }

  uint32_t serial_num;

  result = bosonGetCameraSN(&serial_num);

  if (result)
    {
      perror("[BOSON] Failed to get camera serial number, aborting");
      Close();
      exit(-1);
    }
  else
    {
      serial_number_ = std::to_string(serial_num);
      std::cout << "[BOSON] Talking to camera with serial number: " << serial_number_ << std::endl;
    }
  return serial_number_;
}


cv::Mat Boson::getParams(std::string file_path, std::string data)
{
  cv::FileStorage fs(file_path, FileStorage::READ);
  cv::Mat M;
  fs[data] >> M;

  if (M.rows == 0 || M.cols == 0)
    {
      std::cout << "[BOSON] Unable to load calibration file at: " << file_path << ", returning empty matrix" << std::endl;
      return M;
    }
  else
    {
      std::cout << "[BOSON] Found file with data type " << data <<", load matrix of size: (" << M.rows << "," <<M.cols << ")" << std::endl;
    }
  return M;
}
