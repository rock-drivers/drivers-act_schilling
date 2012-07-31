#include "ActHandler.hpp"
#include <base_schilling/SchillingRaw.hpp>
#include "Error.hpp"
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
  mActData.ctrlMode = config.ctrlMode;
  mActRunState = RESET;
  mUpdateState.statusUpdate = false;
  mUpdateState.posUpdate = false;
  mUpdateState.driveStateUpdate = false;
  mUpdateState.actInfoUpdate = false;
}

void ActHandler::initDevice()
{
  int ctrlMode = (int)mConfig.ctrlMode;
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
  return mMsgQueue.empty();
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
  if(mConfig.ctrlMode == MODE_VEL && mActRunState == RUNNING){
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
  if(mConfig.ctrlMode == MODE_NONE){
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
  if(ctrlMode == mConfig.ctrlMode){
    return;
  }
  if(ctrlMode == MODE_VEL){
    setVelocity(0);
  }
  enqueueCmdMsg(CMD_SETCTRLMODE,ctrlMode,1);
  mConfig.ctrlMode = ctrlMode;
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
   if(mUpdateState.statusUpdate && mUpdateState.posUpdate){
    mUpdateState.statusUpdate=false;
    mUpdateState.posUpdate=false;
    return true;
  }
  return false;
}

bool ActHandler::hasPosUpdate()
{
  if(mUpdateState.posUpdate){
    mUpdateState.posUpdate=false;
    return true;
  }
  return false;
}

bool ActHandler::hasDriveStatusUpdate()
{
  if(mUpdateState.driveStateUpdate){
    mUpdateState.driveStateUpdate=false;
    return true;
  }
  return false;
}

bool ActHandler::hasActInfoUpdate()
{
  if(mUpdateState.actInfoUpdate){
    mUpdateState.actInfoUpdate=false;
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
	return len;
      }
      return 0;
    }
  }
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
	mActDevStatus.ctrlStatus = (*buffer)[2];
	mActDevStatus.driveStatus = (*buffer)[3];
	mActData.ctrlMode = (act_schilling::ControlMode)(*buffer)[4];
	mActDevStatus.shaftPos = (*buffer)[8];
	mActDevStatus.shaftPos |= (*buffer)[7] << 8;
	mActDevStatus.shaftPos |= (*buffer)[6] << 16;
	mActDevStatus.shaftPos |= (*buffer)[5] << 24;
	mActData.shaftAng = count2ang(mActDevStatus.shaftPos);
	int16_t vel =  (*buffer)[10];
	vel |= (*buffer)[9] << 8;
	mActData.shaftVel = double(vel)/ACT_VEL_COEFF;
	if(mActRunState < RUNNING){
	  checkRunState();
	}
	mLastPos.pos = mActDevStatus.shaftPos;
	mUpdateState.statusUpdate = true;
	break;
      }
      case CMD_GETPOS:{
	if (((act_schilling::raw::MsgHeader*)(buffer->data()))->length != 0x0D){
	  throw MarError(MARSTR_DEVREPINV,MARERROR_DEVREPINV);
	}
	mActPosition.time = base::Time::now();
	mActPosition.extEncoderStatus = (*buffer)[2];
	mActPosition.extAbsPos = (*buffer)[4];
	mActPosition.extAbsPos |= (*buffer)[3] << 8;
	mActPosition.shaftPos = (*buffer)[8];
	mActPosition.shaftPos |= (*buffer)[7] << 8;
	mActPosition.shaftPos |= (*buffer)[6] << 16;
	mActPosition.shaftPos |= (*buffer)[5] << 24;
	mActPosition.shaftEncStatus = (*buffer)[9];
	mActPosition.shaftAbsPos = (*buffer)[11];
	mActPosition.shaftAbsPos |= (*buffer)[10] << 8;
	mActDevStatus.encoderStatus = (*buffer)[9];
	mUpdateState.posUpdate = true;
	cout <<"shaftPos: " <<mActPosition.shaftPos <<" shaftAbsPos: " <<mActPosition.shaftAbsPos <<" extAbsPos: " <<mActPosition.extAbsPos <<endl;
	break;
      }
     case CMD_GETDRVSTAT:{
	if (((act_schilling::raw::MsgHeader*)(buffer->data()))->length != 0x0C){
	  throw MarError(MARSTR_DEVREPINV,MARERROR_DEVREPINV);
	}
	mActDriveStatus.time = base::Time::now();
	mActDriveStatus.driveStatus = (*buffer)[2];
	mActDriveStatus.driveProtectStatus = (*buffer)[4];
	mActDriveStatus.driveProtectStatus |= (*buffer)[3] << 8;
	mActDriveStatus.systemProtectStatus = (*buffer)[6];
	mActDriveStatus.systemProtectStatus |= (*buffer)[5] << 8;
	mActDriveStatus.driveSystemStatus1 = (*buffer)[8];
	mActDriveStatus.driveSystemStatus1 |= (*buffer)[7] << 8;
	mActDriveStatus.driveSystemStatus2 = (*buffer)[10];
	mActDriveStatus.driveSystemStatus2 |= (*buffer)[9] << 8;
	mUpdateState.driveStateUpdate = true;
	break;
      }
      case CMD_GETACTINFO:{
	if (((act_schilling::raw::MsgHeader*)(buffer->data()))->length != 0x0C){
	  throw MarError(MARSTR_DEVREPINV,MARERROR_DEVREPINV);
	}
	mActDriveStatus.time = base::Time::now();
	mActInfo.serialNo = (*buffer)[7];
	mActInfo.serialNo |= (*buffer)[6];
	mActInfo.firmwareRev = (*buffer)[8];
	mUpdateState.actInfoUpdate = true;
	break;
      }

      default: break;    
    }
  }
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
  switch (mActRunState){
    case INIT : {
	mActRunState = INITIALIZED;
	mActState.initialized = true;
	break;
    }
    case FINDMIN : {
      if(!checkMoving(mActDevStatus.shaftPos)){
	mActBoundaries.min = mActDevStatus.shaftPos;
	mActRunState = FINDMAX;
	setAnglePos(360,CAL_VEL_COEFF);
      }
      break;
    }
    case FINDMAX : {
      if(!checkMoving(mActDevStatus.shaftPos)){
	mActRunState = SETZERO;
	int range = mActDevStatus.shaftPos - mActBoundaries.min;
	setPos(range/2+mActBoundaries.min);
	mActBoundaries.max = count2ang(range/2);
	mActBoundaries.min = mActBoundaries.max*(-1);		  
      }
      break;
    }
    case SETZERO: {
      if(!checkMoving(mActDevStatus.shaftPos)){
	mActRunState = GOHOME; 
	enqueueCmdMsg(CMD_CLRSHAFTPOS);
	setPos(ang2count(mConfig.homePos));
      }
      break;
    }
    case GOHOME: {
      if(!checkMoving(mActDevStatus.shaftPos)){
	setVelocity(0);
	enqueueCmdMsg(CMD_SETCTRLMODE,mConfig.ctrlMode,1);
	mActRunState = RUNNING;
	mActState.calibrated = true;		
      }
    }
    default: break;
  }
}
