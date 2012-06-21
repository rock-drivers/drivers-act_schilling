#include "ActHandler.hpp"
#include <base_schilling/SchillingRaw.hpp>
#include <iostream>


#define CAL_VEL_COEFF 0.5

using namespace act_schilling;
using namespace act_schilling::raw;
using namespace std;

ActHandler::ActHandler(const Config& config)
  : base_schilling::Driver(64),
    mConfig(config),mLastCmd(CMD_NONE)
{
  mActData.ctrlStatus = -1;
  mActData.driveStatus = -1;
  mActData.ctrlMode = -1;
  mActData.shaftPos = 0;
  mActData.shaftAng = 0;
  mActData.shaftVel = 0;
  mActState.initialized = false;
  mActState.calibrated = false;
  mActRunState = RESET;
  mCalibData.min = 0;
  mCalibData.max = 0;
  mCalibData.zero = 0;
}

void ActHandler::initDevice()
{
  cout <<"initDevice" <<endl;
  int ctrlMode = mConfig.velMode ? act_schilling::raw::MODE_VEL : act_schilling::raw::MODE_POS;
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
}


bool ActHandler::isIdle()
{
  return mMsgQueue.empty();
}

act_schilling::ActData ActHandler::getData() const
{
  return mActData;
}

act_schilling::ActState ActHandler::getState() const
{
  return mActState;
}

void ActHandler::setPos(int count, float velCoeff)
{
  cout <<"setPos " <<count <<endl;
  mLastPos.count = 0;
  enqueueCmdMsg(CMD_CLRERR);
  enqueueCmdMsg(CMD_SETSHAFTPOS,count,4);
  enqueueCmdMsg(CMD_SETVEL,int(float(mConfig.velocity)*velCoeff),4);
  enqueueCmdMsg(CMD_CLRERR);

}

void ActHandler::setAnglePos(int ang, float velCoeff)
{
  cout <<"setAnglePos " <<ang <<endl;
  setPos(ang2count(ang),velCoeff);
}

void ActHandler::setVelocity(int vel)
{
  enqueueCmdMsg(CMD_SETVEL,vel,4);
}

void ActHandler::calibrate()
{
  cout <<"calibrate" <<endl;
  mActRunState = FINDMIN;
  setAnglePos(-360,CAL_VEL_COEFF);
}


void ActHandler::enqueueCmdMsg(CMD cmd,int value, int length)
{
  std::vector<uint8_t> msg(4+length);
  //unsigned char *msg = new unsigned char[4+length];
  act_schilling::raw::MsgHeader *header =  (act_schilling::raw::MsgHeader*)msg.data();
  header->type = SCHILL_CMD_MSG;
  header->length = 4+length;
  header->cmd = cmd;
  for (int i = 0;i<length;i++){
    int shift = (length-1-i)*8;
    msg[i+3] = (value & (0xFF<<shift)) >> shift;
  }
  setCS((char*)msg.data());
  mLastCmd = cmd;
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
      size_t len = ((act_schilling::raw::MsgHeader*)buffer)->length;
      if(buffer_size >= len){
	return len;
      }
      return 0;
    }
  }
  return -buffer_size;
  //TODO: handle other msg
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
    throw std::runtime_error("empty Data");
  }
  act_schilling::raw::MsgHeader  *header = (act_schilling::raw::MsgHeader*)cData;
  int length = header->length;
  char cCs = 0;
  for (int i=0;i<length-1;i++){
    cCs += cData[i];
  }
  if (cCs != cData[length-1]){
    throw std::runtime_error("invalid checksum");
    }
  return;
}

void ActHandler::parseReply(const std::vector<uint8_t>* buffer)
{
  if((*buffer)[0]==ACT_SCHILLING_ACK){
    cout <<"Ack Received" <<endl;
    return;
  }
  else if((*buffer)[0]==ACT_SCHILLING_NAK){
    cout <<"Nak Received" <<endl;
    throw std::runtime_error("Device Nak");
  }
  else if((*buffer)[0]==SCHILL_REPL_UNCHG_MSG ||
    (*buffer)[0]==SCHILL_REPL_CHG_MSG){
      int i = ((act_schilling::raw::MsgHeader*)(buffer->data()))->length;
      cout <<"Reply Received, length: " <<i <<endl;
      switch(mLastCmd){
	case CMD_GETSTAT:{
	  if (((act_schilling::raw::MsgHeader*)(buffer->data()))->length != 0x0C){
	    throw std::runtime_error("invalid reply length");
	  }
	  mActData.ctrlStatus = (*buffer)[2];
	  mActData.driveStatus = (*buffer)[3];
	  mActData.ctrlMode = (*buffer)[4];
	  mActData.shaftPos = (*buffer)[8];
	  mActData.shaftPos |= (*buffer)[7] << 8;
	  mActData.shaftPos |= (*buffer)[6] << 16;
	  mActData.shaftPos |= (*buffer)[5] << 24;
	  mActData.shaftAng = count2ang(mActData.shaftPos);
	  int16_t vel =  (*buffer)[10];
	  vel |= (*buffer)[9] << 8;
	  mActData.shaftVel = vel;
	  //mActData.shaftVel = (*buffer)[10];
	  //mActData.shaftVel |= (*buffer)[9] << 8;
	  cout << "mActRunState : " <<mActRunState <<" position: " <<mActData.shaftPos <<endl;
	  if(mActRunState < RUNNING){
	    switch (mActRunState){
	      case INIT : {
		mActRunState = INITIALIZED;
		mActState.initialized = true;
		break;
	      }
	      case FINDMIN : {
		if(!checkMoving(mActData.shaftPos)){
		  mCalibData.min = mActData.shaftPos;
		  mActRunState = FINDMAX;
		  setAnglePos(360,CAL_VEL_COEFF);
		}
		break;
	      }
	      case FINDMAX : {
		if(!checkMoving(mActData.shaftPos)){
		  mActRunState = SETZERO;
		  int range = mActData.shaftPos - mCalibData.min;
		  setPos(range/2+mCalibData.min);
		  mCalibData.max = range/2;
		  mCalibData.min = mCalibData.max*(-1);		  
		}
		break;
	      }
	      case SETZERO: {
		if(!checkMoving(mActData.shaftPos)){
		  mActRunState = GOHOME; 
		  enqueueCmdMsg(CMD_CLRSHAFTPOS);
		  setPos(ang2count(mConfig.homePos));
		}
		break;
	      }
	      case GOHOME: {
		if(!checkMoving(mActData.shaftPos)){
		  mActRunState = RUNNING;
		  mActState.calibrated = true;		
		}
	      }
	      default: break;
	    }
	  }
	  mLastPos.pos = mActData.shaftPos;
      }break;
      default: break;    
    }
  }
}


int ActHandler::ang2count(int ang)
{
  return int(float(ang)*ACT_FULLPOS/360);
}

int ActHandler::count2ang(int count)
{
  return int(float(count)*360/ACT_FULLPOS);
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
