#ifndef ACT_SCHILLING_ACTTYPES_HPP
#define ACT_SCHILLING_ACTTYPES_HPP

#include <base/time.h>

namespace act_schilling {
   
    /** Control Mode */
    enum ControlMode{
      //! Control mode none, actuator moving functions are disabled
	MODE_NONE = 0,
      //! Control mode position, actuator is controlled by position commands
	MODE_POS,
      //! Control mode velocity, actuator is controlled by velocity commands
	MODE_VEL
    };
    
    /** This structure holds operational data **/
    struct ActData {
	//! timestamp
        base::Time  time; 
	//! control mode
	act_schilling::ControlMode ctrlMode; 
	//! actual position in signed angle
	double shaftAng;  
	//! actual shaft velocity
	double shaftVel;	  
	ActData()
	  : time(base::Time::now()),ctrlMode(MODE_NONE),shaftAng(0),shaftVel(0)
	{}
    };
    
    /** This structure holds status data */
    struct ActDeviceStatus {
	//! timestamp
        base::Time  time; 
	//! control status
	uint8_t ctrlStatus; 
	//! drive status
	uint8_t driveStatus; 
	//! encoder status
	uint8_t encoderStatus; 
	//! shaft position in signed encoder counts
	int shaftPos;	
	ActDeviceStatus() :
	  time(base::Time::now()),ctrlStatus(0),driveStatus(0),encoderStatus(0)
	{}
    };
    
    /** This structure holds actuator run state information*/
    struct ActState {
      //! flag set when actuator is initialized
      bool initialized;
      //! flag set when actuator is calibrated
      bool calibrated;
      ActState()
	: initialized(false),calibrated(false)
      {}
    }; 
    
    /** This structure holds position data, usually not necessary for operational mode */
    struct ActPosition{ 
      //! timestamp
      base::Time time;
      //! external encoder status
      uint8_t extEncoderStatus;
      //! external encoder abs position
      int extAbsPos;
      //! shaft position
      int shaftPos;
      //! shaft encoder status
      uint8_t shaftEncStatus;
      //! shaft absolute position
      int shaftAbsPos;
      ActPosition()
	: time(base::Time::now()),extEncoderStatus(0),extAbsPos(0),shaftPos(0),shaftEncStatus(0)
      {}
    };
    
    /** This structure holds drive status data, usually not necessary for operational mode */
    struct ActDriveStatus{
      //! timestamp
      base::Time time;
      //! drive status
      uint8_t driveStatus;
      //! drive protect status
      uint16_t driveProtectStatus;
      //! system protect status
      uint16_t systemProtectStatus;
      //! drive system status 1
      uint16_t driveSystemStatus1;
      //! drive system status 2
      int driveSystemStatus2;
      ActDriveStatus()
	: time(base::Time::now()),driveStatus(0),driveProtectStatus(0),systemProtectStatus(0),driveSystemStatus1(0),driveSystemStatus2(0)
      {}
    };
    
    /** This structure holds actuator info, usually not necessary for operational mode */
    struct ActInfo{
      //! timestamp
      base::Time time;
      //! serial number
      int serialNo;
      //! firmware revision
      int firmwareRev;
      ActInfo()
	: time(base::Time::now()),serialNo(0),firmwareRev(0)
      {}
    };
    
    /** This structure holds min and max values, evaluated in calibration process */
    struct ActBoundaries{
      //! min value evaluated by calibration process
      double min;
      //! max value evaluated by calibration process
      double max;
      ActBoundaries()
	: min(0),max(0)
      {}
    };
    
    
}


#endif 
