#ifndef _ACT_SCHILLING_CONFIG_HPP_
#define _ACT_SCHILLING_CONFIG_HPP_

#include "ActTypes.hpp"

namespace act_schilling
{

struct Config
{
        int velocity;	
	ControlMode ctrlMode;
	int homePos;
	
	Config()
            : velocity(1250),
	      ctrlMode(MODE_VEL)
        {   
        }   

};

}
#endif