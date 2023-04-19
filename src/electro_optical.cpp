/* Author: Jason Hughes
 * Date: April 2023
 * About: Main file for EO camera interfacing
 */

#include "eeyore/electro_optical.hpp"

ElectroOpticalCam::ElectroOpticalCam( int h, int w, TriggerType t )
{
  setHeight( h );
  setWidth( w );
  setTrigger( t );

  system_ = System::GetInstance();

  CameraList cam_list = system_->GetCameras();

  cam_ = cam_list.GetByIndex(0);
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
	  std::cout << "[EO CAMERA] Software Trigger set" << std::endl;
	}
      else
	{
	  std::cout << "[EO CAMERA] Hardware Trigger set" << std::endl;
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

      cam_ -> TriggerSelector.SetValue(TriggerSelector_FrameStart);

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
      else
	{
	  if (!IsWritable(cam_->TriggerSource))
	    {
	      std::cout << "[EO CAMERA] Unable to set hardware trigger, aborting" << std::endl;
	    }

	  cam_ -> TriggerSource.SetValue(TriggerSource_Line0);

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
      std::cout << "[EO CAMERA] Error: " << e.what() << std::endl;
      return image_final;
    }
  
  return image_final;
}     

void ElectroOpticalCam::closeCamera()
{
  cam_ -> EndAcquisition();
}

cv::Mat ElectroOpticalCam::getParams(std::string file_path, std::string data)
{
  cv::FileStorage fs(file_path, cv::FileStorage::READ);
  cv::Mat M;
  fs[data] >> M;

  return M;
}
