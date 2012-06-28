#ifndef ACT_SCHILLING_ACTRAW_HPP
#define ACT_SCHILLING_ACTRAW_HPP

#define ACT_SCHILLING_ACK 0x06
#define ACT_SCHILLING_NAK 0x15

#define ACT_FULLPOS  205000


#define ACT_ENC_LIN_ALARM 	0x08
#define ACT_ENC_RANGE_ERR 	0x10
#define ACT_CTRL_WD_TIME	0x01
#define ACT_CTRL_EXT_ENC_MAG	0x02
#define ACT_CTRL_EXT_ENC_COMM	0x04
#define ACT_CTRL_SH_ENC_MAG	0x08
#define ACT_CTRL_WATER		0x10
#define ACT_CTRL_SH_ENC_COMM	0x20
#define ACT_DRV_CMD_INC		0x02
#define ACT_DRV_CMD_INV		0x04
#define ACT_DRV_FRAME_ERR	0x08
#define ACT_DRV_VOLT_TEMP	0x10
#define ACT_DRV_COMM_PHASE	0x20



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
      
      struct MsgHeader
      {
	unsigned char type;
	unsigned char length;
	unsigned char cmd;
      };
  }
}

#endif
  
  










