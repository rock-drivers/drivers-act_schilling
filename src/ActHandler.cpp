#include "ActHandler.hpp"
#include <base_schilling/SchillingRaw.hpp>
#include <base_schilling/Error.hpp>
#include <iostream>


#define CAL_VEL_COEFF 0.5

using namespace act_schilling;
using namespace act_schilling::raw;
using namespace std;
using namespace oro_marum;

ActHandler::ActHandler(const Config& config)
  : base_schilling::Driver(64),
    mLastCmd(CMD_NONE), mConfig(config)
{
  mActData.ctrl_mode = config.ctrl_mode;
  mActRunState = RESET;
  mUpdateState.status_update = false;
  mUpdateState.pos_update = false;
  mUpdateState.drive_state_update = false;
  mUpdateState.act_info_update = false;
}

void ActHandler::initDevice()
{
  //cout <<"ActHandler initDevice" <<endl;
  int ctrlMode = (int)mConfig.ctrl_mode;
  mActRunState = INIT;
  mMsgQueue.clear();
  enqueueCmdMsg(CMD_CLRERR);
  enqueueCmdMsg(CMD_CLRERR);
  enqueueCmdMsg(CMD_SETTRAPVEL, 0, 3);
  enqueueCmdMsg(CMD_SETCTRLMODE,ctrlMode,1);
  enqueueCmdMsg(CMD_GETSTAT);  
}

void ActHandler::requestStatus()
{
  enqueueCmdMsg(CMD_GETSTAT);
  enqueueCmdMsg(CMD_GETPOS);
}


bool ActHandler::isIdle()
{
  //return mMsgQueue.empty();
  return !mLastCmd;
}

act_schilling::ActData ActHandler::getData() const
{
  return mActData;
}

ActDeviceStatus ActHandler::getDeviceStatus() const
{
  return mActDevStatus;
}


act_schilling::ActState ActHandler::getState() const
{
  return mActState;
}

void ActHandler::setPos(int count, float velCoeff)
{
  if(mConfig.ctrl_mode == MODE_VEL && mActRunState == RUNNING){
    return;
  }
  if(mActRunState == RUNNING){
    int i = ang2count(mActBoundaries.min);
    if(count<i){
      count = i;
    }
    i = ang2count(mActBoundaries.max);
    if(count>i){
      count = i;
    }
  }
  mLastPos.count = 0;
  enqueueCmdMsg(CMD_CLRERR);
  enqueueCmdMsg(CMD_SETSHAFTPOS,count,4);
  setVelocity(double(mConfig.velocity)*velCoeff);
  enqueueCmdMsg(CMD_CLRERR);
}

void ActHandler::setAnglePos(double ang, double velCoeff)
{
  setPos(ang2count(ang),velCoeff);
}

void ActHandler::setVelocity(double vel)
{
  if(vel<(-ACT_VEL_MAX_RPM)){
    vel = -ACT_VEL_MAX_RPM;
  }  
  if(vel>ACT_VEL_MAX_RPM){
    vel = ACT_VEL_MAX_RPM;
  }
  enqueueCmdMsg(CMD_SETVEL,ACT_VEL_COEFF*vel,4);
}

void ActHandler::calibrate()
{
  if(mConfig.ctrl_mode == MODE_NONE){
    return;
  }
  if(mActRunState > INITIALIZED && mActRunState < RUNNING){
    return;
  }
  mActRunState = FINDMIN;
  enqueueCmdMsg(CMD_SETCTRLMODE,MODE_POS,1);
  setAnglePos(-360,CAL_VEL_COEFF);
}

void ActHandler::setControlMode(ControlMode const ctrlMode)
{
  if(mActRunState > INITIALIZED && mActRunState < RUNNING){
    return;
  }
  if(ctrlMode == mConfig.ctrl_mode){
    return;
  }
  if(ctrlMode == MODE_VEL){
    setVelocity(0);
  }
  enqueueCmdMsg(CMD_SETCTRLMODE,ctrlMode,1);
  mConfig.ctrl_mode = ctrlMode;
}

void ActHandler::requestPosition()
{
  enqueueCmdMsg(CMD_GETPOS);
}

void ActHandler::requestDriveStatus()
{
  enqueueCmdMsg(CMD_GETDRVSTAT);
}

void ActHandler::requestActInfo()
{
  enqueueCmdMsg(CMD_GETACTINFO);
}


ActPosition ActHandler::getPosition()
{
  return mActPosition;
}

ActDriveStatus ActHandler::getDriveStatus()
{
  return mActDriveStatus;
}

ActInfo ActHandler::getActInfo()
{
  return mActInfo;
}

ActBoundaries ActHandler::getBoundaries()
{
  return mActBoundaries;
}

bool ActHandler::hasStatusUpdate()
{
   if(mUpdateState.status_update && mUpdateState.pos_update){
    mUpdateState.status_update=false;
    mUpdateState.pos_update=false;
    return true;
  }
  return false;
}

bool ActHandler::hasPosUpdate()
{
  if(mUpdateState.pos_update){
    mUpdateState.pos_update=false;
    return true;
  }
  return false;
}

bool ActHandler::hasDriveStatusUpdate()
{
  if(mUpdateState.drive_state_update){
    mUpdateState.drive_state_update=false;
    return true;
  }
  return false;
}

bool ActHandler::hasActInfoUpdate()
{
  if(mUpdateState.act_info_update){
    mUpdateState.act_info_update=false;
    return true;
  }
  return false;
}

void ActHandler::setResetState()
{
  mActState.initialized = false;
  mActState.calibrated = false;
  mActRunState = RESET;
}

void ActHandler::enqueueCmdMsg(CMD cmd,int value, int length)
{
  std::vector<uint8_t> msg(4+length);
  act_schilling::raw::MsgHeader *header =  (act_schilling::raw::MsgHeader*)msg.data();
  header->type = SCHILL_CMD_MSG;
  header->length = 4+length;
  header->cmd = cmd;
  for (int i = 0;i<length;i++){
    int shift = (length-1-i)*8;
    msg[i+3] = (value & (0xFF<<shift)) >> shift;
  }
  setCS((char*)msg.data());
  mMsgQueue.push_back(msg);
}

int ActHandler::extractPacket (uint8_t const *buffer, size_t buffer_size) const
{
  //cout <<"ActHandler extractPacket" <<buffer_size <<endl;
  for (size_t i = 0; i < buffer_size; i++) {
    if (buffer[i] == ACT_SCHILLING_ACK)
    {
      if(i){
	return -i;
      }
      return 1;
    }
    else if (buffer[i] == SCHILL_REPL_UNCHG_MSG ||
	buffer[i] == SCHILL_REPL_CHG_MSG)
    {
      if(i){
	return -i;
      }
      if(buffer_size<2){
	return 0;
      }
      size_t len = ((act_schilling::raw::MsgHeader*)buffer)->length;
      if(buffer_size >= len){
	//cout <<"returning len " <<len <<endl;
	return len;
      }
      //cout <<"returning 0" <<endl;
      return 0;
    }
  }
  //cout <<"returning -buffer_size " <<(-buffer_size) <<endl;
  return -buffer_size;
}

void ActHandler::setCS(char *cData)
{
  if (!cData){
    return;
  }
  act_schilling::raw::MsgHeader *header = (act_schilling::raw::MsgHeader*)cData;
  int length = header->length;
  cData[length-1] = 0;
  for (int i=0;i<length-1;i++){
    cData[length-1] += cData[i];
  }
  cData[length-1] = 0x100 - (cData[length-1] & 0xFF);
  return;
}

void ActHandler::checkCS(const char *cData)
{
  if (!cData){
    throw MarError(MARSTR_PARAMINV,MARERROR_PARAMINV);
  }
  act_schilling::raw::MsgHeader  *header = (act_schilling::raw::MsgHeader*)cData;
  int length = header->length;
  char cCs = 0;
  for (int i=0;i<length-1;i++){
    cCs += cData[i];
  }
  cCs = 0x100 - (cCs & 0xFF);
  if (cCs != cData[length-1]){
    throw MarError(MARSTR_CHECKSUM,MARERROR_CHECKSUM);
  }
  return;
}

void ActHandler::parseReply(const std::vector<uint8_t>* buffer)
{
  if((*buffer)[0]==ACT_SCHILLING_ACK){
    //cout <<" ActHandler ACK received" <<endl;  
    mLastCmd = CMD_NONE;
    return;
  }
  else if((*buffer)[0]==ACT_SCHILLING_NAK){
    throw MarError(MARSTR_DEVNAK,MARERROR_DEVNAK);
  }
  else if((*buffer)[0]==SCHILL_REPL_UNCHG_MSG ||
    (*buffer)[0]==SCHILL_REPL_CHG_MSG){
    checkCS((const char*)buffer->data());
    switch(mLastCmd){
      case CMD_GETSTAT:{
	if (((act_schilling::raw::MsgHeader*)(buffer->data()))->length != 0x0C){
	  throw MarError(MARSTR_DEVREPINV,MARERROR_DEVREPINV);
	}
	mActData.time = base::Time::now();
	mActDevStatus.ctrl_status = (*buffer)[2];
	mActDevStatus.drive_status = (*buffer)[3];
	mActData.ctrl_mode = (act_schilling::ControlMode)(*buffer)[4];
	mActDevStatus.shaft_pos = (*buffer)[8];
	mActDevStatus.shaft_pos |= (*buffer)[7] << 8;
	mActDevStatus.shaft_pos |= (*buffer)[6] << 16;
	mActDevStatus.shaft_pos |= (*buffer)[5] << 24;
	mActData.shaft_ang = count2ang(mActDevStatus.shaft_pos);
	int16_t vel =  (*buffer)[10];
	vel |= (*buffer)[9] << 8;
	mActData.shaft_vel = double(vel)/ACT_VEL_COEFF;
	if(mActRunState < RUNNING){
	  checkRunState();
	}
	mLastPos.pos = mActDevStatus.shaft_pos;
	mUpdateState.status_update = true;
	break;
      }
      case CMD_GETPOS:{
	if (((act_schilling::raw::MsgHeader*)(buffer->data()))->length != 0x0D){
	  throw MarError(MARSTR_DEVREPINV,MARERROR_DEVREPINV);
	}
	mActPosition.time = base::Time::now();
	mActPosition.ext_encoder_status = (*buffer)[2];
	mActPosition.ext_abs_pos = (*buffer)[4];
	mActPosition.ext_abs_pos |= (*buffer)[3] << 8;
	mActPosition.shaft_pos = (*buffer)[8];
	mActPosition.shaft_pos |= (*buffer)[7] << 8;
	mActPosition.shaft_pos |= (*buffer)[6] << 16;
	mActPosition.shaft_pos |= (*buffer)[5] << 24;
	mActPosition.shaft_enc_status = (*buffer)[9];
	mActPosition.shaft_abs_pos = (*buffer)[11];
	mActPosition.shaft_abs_pos |= (*buffer)[10] << 8;
	mActDevStatus.encoder_status = (*buffer)[9];
	mUpdateState.pos_update = true;
	//cout <<"shaft_pos: " <<mActPosition.shaft_pos <<" shaft_abs_pos: " <<mActPosition.shaft_abs_pos <<" ext_abs_pos: " <<mActPosition.ext_abs_pos <<endl;
	break;
      }
     case CMD_GETDRVSTAT:{
	if (((act_schilling::raw::MsgHeader*)(buffer->data()))->length != 0x0C){
	  throw MarError(MARSTR_DEVREPINV,MARERROR_DEVREPINV);
	}
	mActDriveStatus.time = base::Time::now();
	mActDriveStatus.drive_status = (*buffer)[2];
	mActDriveStatus.drive_protect_status = (*buffer)[4];
	mActDriveStatus.drive_protect_status |= (*buffer)[3] << 8;
	mActDriveStatus.system_protect_status = (*buffer)[6];
	mActDriveStatus.system_protect_status |= (*buffer)[5] << 8;
	mActDriveStatus.drive_system_status1 = (*buffer)[8];
	mActDriveStatus.drive_system_status1 |= (*buffer)[7] << 8;
	mActDriveStatus.drive_system_status2 = (*buffer)[10];
	mActDriveStatus.drive_system_status2 |= (*buffer)[9] << 8;
	mUpdateState.drive_state_update = true;
	break;
      }
      case CMD_GETACTINFO:{
	if (((act_schilling::raw::MsgHeader*)(buffer->data()))->length != 0x0C){
	  throw MarError(MARSTR_DEVREPINV,MARERROR_DEVREPINV);
	}
	mActDriveStatus.time = base::Time::now();
	mActInfo.serial_no = (*buffer)[7];
	mActInfo.serial_no |= (*buffer)[6];
	mActInfo.firmware_rev = (*buffer)[8];
	mUpdateState.act_info_update = true;
	break;
      }

      default: break;    
    }
  }
  mLastCmd = CMD_NONE;
}


int ActHandler::ang2count(double ang)
{
  return int(ang*ACT_FULLPOS/360);
}

double ActHandler::count2ang(int count)
{
  return double(count)*360/ACT_FULLPOS;
}

bool ActHandler::checkMoving(int pos)
{
  if(pos == mLastPos.pos){
    if(++mLastPos.count >= 5){
      mLastPos.count = 0;
      return false;
    }
  }  
  else{
    mLastPos.count = 0;
  }
  return true;  
}

void ActHandler::checkRunState()
{
  //cout <<"checkRunState " <<mActRunState <<endl;
  switch (mActRunState){
    case INIT : {
	mActRunState = INITIALIZED;
	mActState.initialized = true;
	break;
    }
    case FINDMIN : {
      if(!checkMoving(mActDevStatus.shaft_pos)){
	mActBoundaries.min = mActDevStatus.shaft_pos;
	mActRunState = FINDMAX;
	setAnglePos(360,CAL_VEL_COEFF);
      }
      break;
    }
    case FINDMAX : {
      if(!checkMoving(mActDevStatus.shaft_pos)){
	mActRunState = SETZERO;
	int range = mActDevStatus.shaft_pos - mActBoundaries.min;
	setPos(range/2+mActBoundaries.min);
	mActBoundaries.max = count2ang(range/2);
	mActBoundaries.min = mActBoundaries.max*(-1);		  
      }
      break;
    }
    case SETZERO: {
      if(!checkMoving(mActDevStatus.shaft_pos)){
	mActRunState = GOHOME; 
	enqueueCmdMsg(CMD_CLRSHAFTPOS);
	setPos(ang2count(mConfig.home_pos));
      }
      break;
    }
    case GOHOME: {
      if(!checkMoving(mActDevStatus.shaft_pos)){
	setVelocity(0);
	enqueueCmdMsg(CMD_SETCTRLMODE,mConfig.ctrl_mode,1);
	mActRunState = RUNNING;
	mActState.calibrated = true;		
      }
    }
    default: break;
  }
}
