/* Author: Jason Hughes
 * Date: April 2023
 * About: Main file for EO camera interfacing
 */

#include "eeyore/electro_optical.hpp"
#include "exiv2/basicio.hpp"

ElectroOpticalCam::ElectroOpticalCam( int h, int w, std::string t )
{
  TriggerType trig;

  if (t == "SOFTWARE")
    {
      trig = SOFTWARE;
    }
  else if (t == "HARDWARE_LINE0")
    {
      trig = HARDWARE_LINE0;
    }
  else if (t == "HARDWARE_LINE1")
    {
      trig = HARDWARE_LINE1;
    }
  else if (t == "HARDWARE_LINE2")
    {
      trig = HARDWARE_LINE2;
    }
  else if (t == "HARDWARE_LINE3")
    {
      trig = HARDWARE_LINE3;
    }
  else
    {
      std::cout << "[EO CAMERA] Invalid Trigger set, exiting" << std::endl;
      exit(1);
    }	
  
  setHeight( h );
  setWidth( w );
  setTrigger( trig );

  system_ = System::GetInstance();

  cam_list_ = system_->GetCameras();

  if (cam_list_.GetSize() == 0)
    {
      std::cout << "[EO CAMERA] No Cameras Found, exiting" << std::endl;
      exit(1);
    }
  
  cam_ = cam_list_.GetByIndex(0);
  cam_->Init();
}

void ElectroOpticalCam::setHeight( int h )
{
  height_ = h;
}

void ElectroOpticalCam::setWidth( int w )
{
  width_ = w;
}

void ElectroOpticalCam::setTrigger( TriggerType t )
{
  trig_ = t;
}

void ElectroOpticalCam::setIntrinsicCoeffs( cv::Mat int_coeffs )
{
  intrinsic_coeffs_ = int_coeffs;
}

void ElectroOpticalCam::setDistanceCoeffs( cv::Mat dist_coeffs )
{
  distance_coeffs_ = dist_coeffs;
}

int ElectroOpticalCam::getHeight()
{
  return height_;
}

int ElectroOpticalCam::getWidth()
{
  return width_;
}

TriggerType ElectroOpticalCam::getTrigger()
{
  return trig_;
}

cv::Mat ElectroOpticalCam::getIntrinsicCoeffs()
{
  return intrinsic_coeffs_;
}

cv::Mat ElectroOpticalCam::getDistanceCoeffs()
{
  return distance_coeffs_;
}

int ElectroOpticalCam::configureTrigger()
{
  int result = 0;
  
  try
    {
      if (trig_ == SOFTWARE)
	{
	  std::cout << "[EO CAMERA] Configuring Software Trigger" << std::endl;
	}
      else
	{
	  std::cout << "[EO CAMERA] Configuring Hardware Trigger" << std::endl;
	}

      if (!IsWritable(cam_->TriggerMode))
	{
	  std::cout << "[EO CAMERA] Unable to disable trigger mode aborting" << std::endl;
	}

      cam_ -> TriggerMode.SetValue(TriggerMode_Off);

      std::cout << "[EO CAMERA] Trigger is ready to be set" << std::endl;

      if (!IsWritable(cam_->TriggerSelector))
	{
	  std::cout << "[EO CAMERA] Unable to set trigger selector, aborting" << std::endl;
	  return -1;
	}

      std::cout << "[EO CAMERA] Trigger selector set to frame start" << std::endl;

      if (trig_ == SOFTWARE)
	{
	  if (!IsWritable(cam_->TriggerSource))
	    {
	      std::cout << "[EO CAMERA] Unable to set software trigger, aborting" << std::endl;
	      return -1;
	    }

	  cam_ -> TriggerSource.SetValue(TriggerSource_Software);

	  std::cout << "[EO CAMERA] Trigger source set to software" << std::endl;
	}
      else if (trig_ == HARDWARE_LINE0)
	{
	  if (!IsWritable(cam_->TriggerSource))
	    {
	      std::cout << "[EO CAMERA] Unable to set hardware trigger, aborting" << std::endl;
	    }

	  cam_ -> TriggerSource.SetValue(TriggerSource_Line0);

	  std::cout << "[EO CAMERA] Trigger source set to hardware" << std::endl;
	}
      else if (trig_ == HARDWARE_LINE1)
	{
	  if (!IsWritable(cam_->TriggerSource))
	    {
	      std::cout << "[EO CAMERA] Unable to set hardware trigger, aborting" << std::endl;
	    }

	  cam_ -> TriggerSource.SetValue(TriggerSource_Line1);

	  std::cout << "[EO CAMERA] Trigger source set to hardware" << std::endl;
	}
      else if (trig_ == HARDWARE_LINE2)
	{
	  if (!IsWritable(cam_->TriggerSource))
	    {
	      std::cout << "[EO CAMERA] Unable to set hardware trigger, aborting" << std::endl;
	    }

	  cam_ -> TriggerSource.SetValue(TriggerSource_Line2);

	  std::cout << "[EO CAMERA] Trigger source set to hardware" << std::endl;
	}
      else if (trig_ == HARDWARE_LINE3)
	{
	  if (!IsWritable(cam_->TriggerSource))
	    {
	      std::cout << "[EO CAMERA] Unable to set hardware trigger, aborting" << std::endl;
	    }

	  cam_ -> TriggerSource.SetValue(TriggerSource_Line3);

	  std::cout << "[EO CAMERA] Trigger source set to hardware" << std::endl;
	}

      if (!IsWritable(cam_->TriggerMode))
	{
	  std::cout << "[EO CAMERA] Unable to disable trigger mode" << std::endl;
	  return -1;
	}

      cam_ -> TriggerMode.SetValue(TriggerMode_On);
    }
  catch (Spinnaker::Exception& e)
    {
      std::cout << "[EO CAMERA] Error: " << e.what() << std::endl;
      result = -1;
    }

  return result;
}

int ElectroOpticalCam::setupCamera()
{
  int result = 0;

  try
    {
      if (!IsWritable(cam_->AcquisitionMode))
	{
	  std::cout << "[EO CAMERA] Unable to set aquisition mode to continuous" << std::endl;
	  return -1;
	}

      cam_ -> AcquisitionMode.SetValue(AcquisitionMode_Continuous);
      std::cout << "[EO CAMERA] Acquisition mode set to continuous" << std::endl;

      processor_.SetColorProcessing(SPINNAKER_COLOR_PROCESSING_ALGORITHM_HQ_LINEAR);
      
    }
  catch (Spinnaker::Exception& e)
    {
      std::cout << "[EO CAMERA] Error: " << e.what() << std::endl;
      result = -1;
    }

  return result;
}

int ElectroOpticalCam::startCamera()
{
  int result = 0;

  try
    {
      cam_ -> BeginAcquisition();
      std::cout << "[EO CAMERA] Camera has started" << std::endl;
    }
  catch (Spinnaker::Exception& e)
    {
      std::cout << "[EO CAMERA] Error: " << e.what() << std::endl;
      result = -1;
    }

  return result;
}	
  
cv::Mat ElectroOpticalCam::getFrame()
{
  int result = 0;
  cv::Mat image_final;
  cv::Mat cv_image;
  
  processor_.SetColorProcessing(SPINNAKER_COLOR_PROCESSING_ALGORITHM_HQ_LINEAR);

  try
    {
      if (trig_ == SOFTWARE)
	{
	  if (!IsWritable(cam_->TriggerSoftware))
	    {
	      std::cout << "Unable to execute software trigger" << std::endl;
	      return image_final;
	    }
	  cam_ -> TriggerSoftware.Execute();
	}

      ImagePtr image_result = cam_ -> GetNextImage(1000);

      if (image_result->IsIncomplete())
	{
	  std::cout << "Image incomplete with status " << image_result->GetImageStatus() << "..." << std::endl;
	}
            
      ImagePtr image_converted = processor_.Convert(image_result, PixelFormat_BGR8);

      int h = image_converted->GetHeight();
      int w = image_converted->GetWidth();
      
      cv_image = cv::Mat(h, w, CV_8UC3, image_converted->GetData(), image_converted->GetStride());
      cv::undistort(cv_image, image_final, intrinsic_coeffs_, distance_coeffs_);
      
      image_result -> Release();
    }
  catch (Spinnaker::Exception& e)
    {
      std::cout << "[EO CAMERA] Error getting frame: " << e.what() << std::endl;
      return image_final;
    }

  return image_final;
}


int ElectroOpticalCam::writeFrame(std::string filename)
{
  int result = 0;

  processor_.SetColorProcessing(SPINNAKER_COLOR_PROCESSING_ALGORITHM_HQ_LINEAR);

  std::ostringstream f_name;

  f_name << filename;
  
  try
    {
      if (trig_ == SOFTWARE)
	{
	  if (!IsWritable(cam_->TriggerSoftware))
	    {
	      std::cout << "Unable to execute software trigger" << std::endl;
	    }
	  cam_ -> TriggerSoftware.Execute();
	}

      ImagePtr image_result = cam_ -> GetNextImage(1000);

      if (image_result->IsIncomplete())
	{
	  std::cout << "Image incomplete with status " << image_result->GetImageStatus() << "..." << std::endl;
	}
            
      ImagePtr image_converted = processor_.Convert(image_result, PixelFormat_BGR8);

      image_converted -> Save(f_name.str().c_str());
    }
  catch (Spinnaker::Exception& e)
    {
      std::cout << "[EO CAMERA] Error writing frame: " << e.what() << std::endl;
      return -1;
    }

  return result;
}


cv::Mat ElectroOpticalCam::getParams(std::string file_path, std::string data)
{
  cv::FileStorage fs(file_path, cv::FileStorage::READ);
  cv::Mat M;
  fs[data] >> M;

  if (M.rows == 0 || M.cols == 0)
    {
      std::cout << "[EO CAMERA] Unable to load calibration file at: " << file_path << ", returning empty matrix" << std::endl;
      return M;
    }
  else
    {
      std::cout << "[EO CAMERA] Found file with data type " << data <<", load matrix of size: (" << M.rows << "," <<M.cols << ")" << std::endl;
    }
  return M;
}

void ElectroOpticalCam::closeDevice()
{
  
  cam_ -> EndAcquisition();
  cam_ -> DeInit();
  delete cam_;
  
  cam_list_.Clear();
  system_ -> ReleaseInstance();
}

void ElectroOpticalCam::restartDevice()
{
  std::cout << "[EO CAMERA] Restarting Camera" << std::endl;
  cam_ -> EndAcquisition();
  cam_ -> DeInit();
  std::cout << "[EO CAMERA] deinit" << std::endl;
  //delete cam_;
  system_ -> UpdateCameras();
  //delete system_;
  std::cout << "[EO CAMERA] 1" << std::endl;
  //system_ = System::GetInstance();
  //cam_list_ = system_->GetCameras();
  std::cout << "[EO CAMERA] 2" << std::endl;
  if (cam_list_.GetSize() == 0)
    {
      std::cout << "[EO CAMERA] No Cameras Found while restarting, exiting" << std::endl;
      exit(1);
    }

  cam_ = cam_list_.GetByIndex(0);
  std::cout << "[EO CAMERA] 3" << std::endl;
  cam_ -> Init();
  std::cout << "[EO CAMERA] Initialized camera on restart" << std::endl;
  configureTrigger();
  setupCamera();
  startCamera();
}

void ElectroOpticalCam::printDeviceInfo()
{
  try
    {
      INodeMap& map = cam_ -> GetTLDeviceNodeMap();

      FeatureList_t features;

      CCategoryPtr category = map.GetNode("DeviceInformation");

      if (IsReadable(category))
	{
	  category -> GetFeatures(features);

	  FeatureList_t::const_iterator it;
	  std::cout << "[EO CAMERA] Printing Device Info" << std::endl;
	  
	  for (it = features.begin(); it != features.end(); ++it)
	    {
	      CNodePtr feature_node = *it;
	      std::cout << "[EO CAMERA] " << feature_node->GetName() << " ";
	      CValuePtr value = (CValuePtr)feature_node;
	      std::cout << (IsReadable(value) ? value->ToString() : "Node not readable") << std::endl;
	    }
	}
      else
	{
	  std::cout << "[EO CAMERA] Device information not readable" << std::endl;
	}     
    }
  catch (Spinnaker::Exception& e)
    {
      std::cout << "Error: " << e.what() << std::endl;
    }
}


std::string ElectroOpticalCam::getSerialNumberFromCam()
{
  std::string serial_number;
  try
    {
      if (IsReadable(cam_->DeviceSerialNumber))
	{
	  serial_number = cam_ -> DeviceSerialNumber.GetValue();
	  std::cout << "[EO CAMERA] Talking to camera with serial number: " << serial_number << std::endl;
	}
      else
	{
	  std::cout << "[EO CAMERA] Cant acquire serial number from camera" << std::endl;
	}
    }
  catch (Spinnaker::Exception& e)
    {
      std::cout << "Error: " << e.what() << std::endl;
    }

  serial_number_ = serial_number;

  return serial_number;
}
	
     
	


    
