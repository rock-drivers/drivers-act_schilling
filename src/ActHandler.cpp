#include "ActHandler.hpp"
#include <base_schilling/SchillingRaw.hpp>
#include <iostream>


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
  mActData.shaftVel = 0;
}

void ActHandler::initDevice()
{
  cout <<"initDevice" <<endl;
  int ctrlMode = mConfig.velMode ? act_schilling::raw::MODE_VEL : act_schilling::raw::MODE_POS;
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
	  mActData.shaftVel = (*buffer)[10];
	  mActData.shaftVel |= (*buffer)[9] << 8;	
      }
    }
  }
  
}
