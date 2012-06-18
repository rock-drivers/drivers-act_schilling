#ifndef ACT_SCHILLING_UCMRAW_HPP
#define ACT_SCHILLING_UCMRAW_HPP

#define ACT_SCHILLING_ACK 0x06
#define ACT_SCHILLING_NAK 0x15

namespace act_schilling {
  namespace raw {
    
     enum CMD {
	CMD_NONE	= 0x00,
	CMD_CLRERR	= 0x03,
	CMD_SETCTRLMODE	= 0x04,
	CMD_GETACTINFO	= 0x05,
	CMD_GETOLDSTAT	= 0x06,
	CMD_CLRSHAFTPOS	= 0x07,
	CMD_SETWD	= 0x09,
	CMD_SETTRAPVEL	= 0x0C,	
	CMD_SETTRAPPOS	= 0x0D,	
	CMD_GETPOS	= 0x20,
	CMD_GETSTAT	= 0x21,
	CMD_GETDRVSTAT	= 0x22,
	CMD_SETVEL	= 0x30,
	CMD_SETSHAFTPOS	= 0x31	
      };
      
      enum CtrlMode{
	MODE_NONE = 0,
	MODE_POS,
	MODE_VEL
      };
      
      struct MsgHeader
      {
	unsigned char type;
	unsigned char length;
	unsigned char cmd;
      };
  }
}

#endif
  
  










