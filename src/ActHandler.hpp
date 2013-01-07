#ifndef _ACT_SCHILLING_ACTHANDLER_HPP_
#define _ACT_SCHILLING_ACTHANDLER_HPP_
#include <deque>
#include <base_schilling/Driver.hpp>
#include "ActRaw.hpp"
#include "Config.hpp"
#include "ActTypes.hpp"

namespace act_schilling
{
  
  class ActHandler : public base_schilling::Driver
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
  
    struct LastPos{
      int pos;
      int count;
    };
  
    struct UpdateState{
      bool status_update;
      bool pos_update;
      bool drive_state_update;
      bool act_info_update;
    };
  
    public:
      ActHandler(const Config& config = Config());
      /** initializes the device, call this first to start communication with the actuator
         *
      */
      virtual void initDevice();
      /** request the device status, call this periodically, check response update with hasStatusUpdate and read requested data with getData and getDeviceStatus
         *
      */
      virtual void requestStatus();
      /** checks if messages to the device are in the queue
         *  @return true: device message queue is empty
      */
      virtual bool isIdle();
      /** get Actuator Data
         * use this to get the control mode, position and velocity, call it in cycles with requestStatus
      */
      ActData getData() const;
      /** get Actuator Device Status
         * use this to get the status bytes and the position in encodeer counts, call it in cycles with requestStatus
      */
      ActDeviceStatus getDeviceStatus() const;
      /** get the Driver and Actuator State
         * only if both flags are set, it is safe to move the actuator, call initDevice and calibrate to get the actuator in the right state to run it (calibration takes some time)
      */      
      ActState getState() const;
      /** set actuator position in encoder counts, only comes into effect when actuator is in position mode
       * @arg count: signed encoder count 
       * @arg velCoeff: coefficient to adjust velocity preset by config
      */
      void setPos(int count, float velCoeff = 1);
      /** set actuator position in angle, only comes into effect when actuator is in position mode
       * @arg count: signed angle
       * @arg velCoeff: coefficient to adjust velocity preset by config
      */
      void setAnglePos(double ang, double velCoeff = 1);
      /** set actuator velocity, if actuator is in velocity mode, actuator starts moving with specified velocity
       * if actuator is in position mode call comes only into effect if actuator is moving
       * @arg vel: velocity from 0 to 960000 RPM
       * @arg velCoeff: coefficient to adjust velocity preset by config
      */
      void setVelocity(double vel);
      /** start calibration process, if calibration is completed calibration flag of ActState given by getState is set 
       * during calibration process calling this method has no effect
      */
      void calibrate();
      /** set the Control Mode to position, velocity or none (actuator is disabled)
       * during calibration process calling this method has no effect
       * @arg mode: control mode
      */
      void setControlMode(act_schilling::ControlMode const mode);
      /** request Position as defined by Schilling Actuator Command List, usually not needed for operational mode
       * call hasPosUpdate to see if response has been processed and getPosition to receive data
      */
      void requestPosition();
      /** request Drive Status as defined by Schilling Actuator Command List, usually not needed for operational mode
       * call hasDriveStateUpdate to see if response has been processed and getDriveStatus to receive data
      */
      void requestDriveStatus();
      /** request Actuator Info as defined by Schilling Actuator Command List, usually not needed for operational mode
       * call hasActInfoUpdate to see if response has been processed and getActInfo to receive data
      */
      void requestActInfo();
      /** get Position as defined by Schilling Actuator Command List, usually not needed for operational mode
       * data is valid if requestPosition has been called and hasPosUpdate returned true
      */
      ActPosition getPosition();      
      /** get Drive Status as defined by Schilling Actuator Command List, usually not needed for operational mode
       * data is valid if requestDriveStatus has been called and hasDriveStateUpdate returned true
      */
      ActDriveStatus getDriveStatus();
      /** get Actuator Info as defined by Schilling Actuator Command List, usually not needed for operational mode
       * data is valid if requestActInfo has been called and hasActInfoUpdate returned true
      */
      ActInfo getActInfo();
      /** get boundaries, i.e. min and max angles defined by the mechanical assembly of the actuator
      */
      ActBoundaries getBoundaries();
      /** call this after status has been requested by requestStatus
       * @return returns true if status available with getData and getDeviceStatus has been updated
      */
      bool hasStatusUpdate();
      /** call this after status has been requested by requestPosition
       * @return returns true if position available with getPosition has been updated
      */
      bool hasPosUpdate();
      /** call this after status has been requested by requestDriveStatus
       * @return returns true if position available with getDriveStatus has been updated
      */
      bool hasDriveStatusUpdate();
      /** call this after actuator info has been requested by requestActInfo
       * @return returns true if actuator info available with getActInfo has been updated
      */
      bool hasActInfoUpdate();
      /** call this to switch driver software into reset state, e.g. after a communication loss or power reset of the device
       * 
      */
      void setResetState();
    protected:
      void enqueueCmdMsg(raw::CMD cmd,int value = 0, int length = 0);
      int extractPacket (uint8_t const *buffer, size_t buffer_size) const;
      virtual void setCS(char *cData);
      virtual void checkCS(const char *cData);
      virtual void parseReply(const std::vector<uint8_t>* buffer);
      int ang2count(double ang);
      double count2ang(int count);
      bool checkMoving(int pos);
      std::deque<std::vector<uint8_t> > mMsgQueue;
      raw::CMD mLastCmd;
    private:
      void checkRunState();
      Config mConfig;
      ActData mActData;
      ActDeviceStatus mActDevStatus;
      ActState mActState;
      ActRunState mActRunState;
      LastPos mLastPos;
      ActPosition mActPosition;
      ActDriveStatus mActDriveStatus;
      ActInfo mActInfo;
      ActBoundaries mActBoundaries;
      UpdateState mUpdateState;
  };
}

#endif