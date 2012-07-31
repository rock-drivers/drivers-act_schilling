#ifndef MARUM_ORO_ERROR_HPP
#define MARUM_ORO_ERROR_HPP

#include <stdexcept>
#include <base/time.h>

#define MARALARM_BASE			0x1000

#define ACTALARM_BASE			0x2000
#define UCMALARM_BASE			0x3000

#define MARERROR_BASE			0x5000

#define MARALARM_COMMS			MARALARM_BASE +  1

#define MARERROR_CHECKSUM		MARERROR_BASE +  1
#define MARERROR_DEVNAK			MARERROR_BASE +  2
#define MARERROR_PARAMINV		MARERROR_BASE +  3
#define MARERROR_DEVREPINV		MARERROR_BASE +  4

#define ACTALARM_ENC_LIN_ALARM		ACTALARM_BASE +  1
#define ACTALARM_ENC_RANGE_ERR 		ACTALARM_BASE +  2
#define ACTALARM_CTRL_WD_TIME		ACTALARM_BASE +  3
#define ACTALARM_CTRL_EXT_ENC_MAG	ACTALARM_BASE +  4
#define ACTALARM_CTRL_EXT_ENC_COMM	ACTALARM_BASE +  5
#define ACTALARM_CTRL_SH_ENC_MAG	ACTALARM_BASE +  6 
#define ACTALARM_CTRL_WATER		ACTALARM_BASE +  7
#define ACTALARM_CTRL_SH_ENC_COMM	ACTALARM_BASE +  8
#define ACTALARM_DRV_CMD_INC		ACTALARM_BASE +  9
#define ACTALARM_DRV_CMD_INV		ACTALARM_BASE + 10
#define ACTALARM_DRV_FRAME_ERR		ACTALARM_BASE + 11
#define ACTALARM_DRV_VOLT_TEMP		ACTALARM_BASE + 12

#define UCMALARM_AD			UCMALARM_BASE + 1
#define UCMALARM_DA			UCMALARM_BASE + 2
#define UCMALARM_EEPROM			UCMALARM_BASE + 3



#define MARSTR_COMMS			"Communication failure"

#define MARSTR_CHECKSUM			"checksum error"
#define MARSTR_DEVNAK			"device Nak"
#define MARSTR_PARAMINV			"invalid param"
#define MARSTR_DEVREPINV		"invalid device response"

#define ACTSTR_ENC_LIN_ALARM		"Encoder Linearity Alarm"
#define ACTSTR_ENC_RANGE_ERR 		"Encoder Out of Range Error"
#define ACTSTR_CTRL_WD_TIME		"Controller Watchdog Timeout"
#define ACTSTR_CTRL_EXT_ENC_MAG		"Controller Ext. Enc. Magnet Loss"
#define ACTSTR_CTRL_EXT_ENC_COMM	"Controller Ext. Enc. Comm Fail"
#define ACTSTR_CTRL_SH_ENC_MAG		"Controller Shaft Enc. Magnet Loss"
#define ACTSTR_CTRL_WATER		"Controller Water Detection"
#define ACTSTR_CTRL_SH_ENC_COMM		"Controller Shaft Enc. Comm Fail"
#define ACTSTR_DRV_CMD_INC		"Drive Cmd Incomplete"
#define ACTSTR_DRV_CMD_INV		"Drive Invalid Cmd"
#define ACTSTR_DRV_FRAME_ERR		"Drive Frame Error"
#define ACTSTR_DRV_VOLT_TEMP		"Drive U/O Volt or Temp"

#define UCMSTR_AD			"AD Failure"
#define UCMSTR_DA			"DA Failure"
#define UCMSTR_EEPROM			"EEPROM Failure"


namespace oro_marum{
  
    enum LogLevel {
        None, Debug, Notification, Warning, Error, Alarm, Fatal
    };
    
    class MarError : public std::runtime_error
    {
      	int mId;
	LogLevel mLogLevel;
      public:
	MarError (const std::string& what, const int& id = 0, const LogLevel& level = Error)
	  : runtime_error(what),mId(id),mLogLevel(level)
	{}
	int id(){
	  return mId;
	}
	LogLevel level(){
	  return mLogLevel;
	}
    };
    
    struct LogMessage {
        base::Time  	time;
        LogLevel       	log_level;
	int 		errorId;
        std::string message;
        LogMessage()
            : log_level(None) {};
        LogMessage(LogLevel level, std::string msg, int error = 0)
            : log_level(level), errorId(error)
        {
            time = base::Time::now();
            message = msg;
        }
        LogMessage(std::runtime_error& error)
	{
	  time = base::Time::now();
	  message = error.what();
	  MarError *marError = dynamic_cast<MarError*>(&error);
	  if(marError){
	    log_level = marError->level();
	    errorId = marError->id();
	  }
	  else{
	    log_level = Debug;
	    errorId = 0;
	  }
	}
    };
    
    
}


#endif
