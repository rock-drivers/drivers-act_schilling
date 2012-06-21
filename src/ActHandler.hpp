#ifndef _ACT_SCHILLING_ACTHANDLER_HPP_
#define _ACT_SCHILLING_ACTHANDLER_HPP_
#include <deque>
#include <base_schilling/Driver.hpp>
#include "ActRaw.hpp"
#include "Config.hpp"
#include "ActTypes.hpp"

namespace act_schilling
{
  
  enum ActRunState{
    RESET,
    INIT,
    INITIALIZED,
    FINDMIN,
    FINDMAX,
    SETZERO,
    GOHOME,
    RUNNING
  };
  
  struct CalibData{
    int min;
    int max;
    int zero;
  };
  
  struct LastPos{
    int pos;
    int count;
  };
  
  class ActHandler : public base_schilling::Driver
  {
    public:
      ActHandler(const Config& config = Config());
      virtual void initDevice();
      virtual void requestStatus();
      virtual bool isIdle();
      ActData getData() const;
      ActState getState() const;
      void setPos(int count, float velCoeff = 1);
      void setAnglePos(int ang, float velCoeff = 1);
      void setVelocity(int vel);
      void calibrate();
      void setControlMode(ControlMode ctrlMode);
      ActPosition getPosition();
      ActDriveStatus getDriveStatus();
      ActInfo getActInfo();
    protected:
      void enqueueCmdMsg(raw::CMD cmd,int value = 0, int length = 0);
      int extractPacket (uint8_t const *buffer, size_t buffer_size) const;
      virtual void setCS(char *cData);
      virtual void checkCS(const char *cData);
      virtual void parseReply(const std::vector<uint8_t>* buffer);
      int ang2count(int ang);
      int count2ang(int count);
      bool checkMoving(int pos);
      virtual void writeNext() = 0;
      std::deque<std::vector<uint8_t> > mMsgQueue;
      raw::CMD mLastCmd;
    private:
      void checkRunState();
      Config mConfig;
      ActData mActData;
      ActState mActState;
      ActRunState mActRunState;
      LastPos mLastPos;
      CalibData mCalibData;    
      ActPosition mActPosition;
      ActDriveStatus mActDriveStatus;
      ActInfo mActInfo;
  };
}

#endif