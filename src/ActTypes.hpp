#ifndef ACT_SCHILLING_UCMTYPES_HPP
#define ACT_SCHILLING_UCMTYPES_HPP

#include <base/time.h>

namespace act_schilling {
    // namespace ctd_seabird

    struct ActData {
        base::Time  time;
	uint8_t ctrlMode;
	double shaftAng;
	int shaftVel;
    };
    
    struct ActDeviceStatus {
        base::Time  time;
	uint8_t ctrlStatus;
	uint8_t driveStatus;
	uint8_t encoderStatus;
	int shaftPos;	
    };
    
    struct ActState {
      bool initialized;
      bool calibrated;
    }; 
    
    struct ActPosition{ 
      base::Time time;
      int encoderStatus;
      int extAbsPos;
      int shaftPos;
      int shaftEncStatus;
      int shaftAbsPos;
    };
    
    struct ActDriveStatus{
      base::Time time;
      int driveStatus;
      int driveProtectStatus;
      int systemProtectStatus;
      int driveSystemStatus1;
      int driveSystemStatus2;
    };
    
    struct ActInfo{
      base::Time time;
      int serialNo;
      int firmwareRev;
    };
    
    struct ActBoundaries{
      double min;
      double max;
    };
    
    enum ControlMode{
	MODE_NONE = 0,
	MODE_POS,
	MODE_VEL
    };
}


#endif 
