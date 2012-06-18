#ifndef ACT_SCHILLING_UCMTYPES_HPP
#define ACT_SCHILLING_UCMTYPES_HPP

#include <base/time.h>

namespace act_schilling {
    // namespace ctd_seabird

    struct ActData {
        base::Time  time;
	uint8_t ctrlStatus;
	uint8_t driveStatus;
	uint8_t ctrlMode;
	int shaftPos;
	int shaftVel;
    };
    
}


#endif 
