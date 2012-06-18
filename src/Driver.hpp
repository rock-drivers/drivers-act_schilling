#ifndef _ACT_SCHILLING_DRIVER_HPP_
#define _ACT_SCHILLING_DRIVER_HPP_

#include "ActHandler.hpp"


namespace act_schilling
{
	class Driver : public ActHandler
	{
	 public: 
	    Driver(const Config& config = Config());
		  
	    /** Read available packets on the I/O
	    *
	    * */
	    void read();

	    /** write next package in queue if available
	    *  writes only if last command sequence has finished or discarded
	    */
	    void writeNext();
	    
	    		
			
	};

} // end namespace 

#endif 
