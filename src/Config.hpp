#ifndef _ACT_SCHILLING_CONFIG_HPP_
#define _ACT_SCHILLING_CONFIG_HPP_

namespace act_schilling
{
  
  
struct Config
{
        int trapVel;	
	bool velMode;
	
	Config()
            : trapVel(20000),
	      velMode(true)
        {   
        }   

};

}
#endif