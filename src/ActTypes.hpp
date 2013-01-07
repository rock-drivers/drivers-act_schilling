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
	act_schilling::ControlMode ctrl_mode; 
	//! actual position in signed angle
	double shaft_ang;  
	//! actual shaft velocity
	double shaft_vel;	  
	ActData()
	  : time(base::Time::now()),ctrl_mode(MODE_NONE),shaft_ang(0),shaft_vel(0)
	{}
    };
    
    /** This structure holds status data */
    struct ActDeviceStatus {
	//! timestamp
        base::Time  time; 
	//! control status
	uint8_t ctrl_status; 
	//! drive status
	uint8_t drive_status; 
	//! encoder status
	uint8_t encoder_status; 
	//! shaft position in signed encoder counts
	int shaft_pos;	
	ActDeviceStatus() :
	  time(base::Time::now()),ctrl_status(0),drive_status(0),encoder_status(0)
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
      uint8_t ext_encoder_status;
      //! external encoder abs position
      int ext_abs_pos;
      //! shaft position
      int shaft_pos;
      //! shaft encoder status
      uint8_t shaft_enc_status;
      //! shaft absolute position
      int shaft_abs_pos;
      ActPosition()
	: time(base::Time::now()),ext_encoder_status(0),ext_abs_pos(0),shaft_pos(0),shaft_enc_status(0),shaft_abs_pos(0)
      {}
    };
    
    /** This structure holds drive status data, usually not necessary for operational mode */
    struct ActDriveStatus{
      //! timestamp
      base::Time time;
      //! drive status
      uint8_t drive_status;
      //! drive protect status
      uint16_t drive_protect_status;
      //! system protect status
      uint16_t system_protect_status;
      //! drive system status 1
      uint16_t drive_system_status1;
      //! drive system status 2
      int drive_system_status2;
      ActDriveStatus()
	: time(base::Time::now()),drive_status(0),drive_protect_status(0),system_protect_status(0),drive_system_status1(0),drive_system_status2(0)
      {}
    };
    
    /** This structure holds actuator info, usually not necessary for operational mode */
    struct ActInfo{
      //! timestamp
      base::Time time;
      //! serial number
      int serial_no;
      //! firmware revision
      int firmware_rev;
      ActInfo()
	: time(base::Time::now()),serial_no(0),firmware_rev(0)
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
