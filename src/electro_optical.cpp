/* Author: Jason Hughes
 * Date: April 2023
 * About: Main file for EO camera interfacing
 */

#include "eeyore/electro_optical.hpp"

ElectroOpticalCam::ElectroOpticalCam( int h, int w )
{
  setHeight( h );
  setWidth( w );

  system_ = System::GetInstance();

  CameraList cam_list = system_->GetCameras();

  cam_->cam_list.GetByIndex(0);

}

void ElectroOpticalCam::setHeight( int h )
{
  height_ = h;
}

void ElectroOpticalCam::setWidth( int w )
{
  width_ = w;
}

int ElectroOpticalCam::getHeight()
{
  return height_;
}

int ElectroOpticalCam::getWidth()
{
  return width_;
}

int ElectroOpticalCam::configureTrigger( TriggerType trig )
{
  int result = 0;
  
  try
    {
      if (trig == SOFTWARE)
	{
	  std::cout << "[EO CAMERA] Software Trigger set" << std::endl;
	}
      else
	{
	  std::cout << "[EO CAMERA] Hardware Trigger set" << std::endl;
	}

      if (!IsWritable(cam_->TriggerMode))
	{
	  std::cout << "[EO CAMERA] Uanble to disable trigger mode aborting" << std::endl;
	}

      cam_ -> TriggerMode.SetValue(TriggerMode_OFF);

      std::cout << "[EO CAMERA] Trigger is ready to be set" << std::endl;

      if (!IsWritable(cam_->TriggerSelector))
	{
	  std::cout << "[EO CAMERA] Unable to set trigger selector, aborting" << std::endl;
	  return -1;
	}

      cam_ -> TriggerSelector.SetValue(TriggerSelector_FrameStart);

      std::cout << "[EO CAMERA] Trigger selector set to frame start" << std::endl;

      if (trig == SOFTWARE)
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

int setupCamera()
{
  int result = 0;

  try
    {
      if (!IsWritable(cam_->AcquisitionMode))
	{
	  std::cout << "[EO CAMERA] Unable to set aquisition mode to continuous" << std::endl;
	  return -1;
	}

      cam_ = AcquisitionMode.SetValue(AcquisitionMode_Continuous);
    }

  return result;
}
	    
  
int acquireImage()
{
  int result = 0;

  return result;
}      
