#ifndef _ACT_SCHILLING_CONFIG_HPP_
#define _ACT_SCHILLING_CONFIG_HPP_

#include "ActTypes.hpp"

namespace act_schilling
{

struct Config
{
        int velocity;	
	ControlMode ctrl_mode;
	int home_pos;
	
	Config()
            : velocity(1250),
	      ctrl_mode(MODE_VEL),
	      home_pos(0)
        {   
        }   

};

}
#endif