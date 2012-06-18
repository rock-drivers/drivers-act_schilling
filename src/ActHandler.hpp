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
    public:
      ActHandler(const Config& config = Config());
      virtual void initDevice();
      virtual void requestStatus();
      virtual bool isIdle();
      ActData getData() const;
    protected:
      void enqueueCmdMsg(raw::CMD cmd,int value = 0, int length = 0);
      int extractPacket (uint8_t const *buffer, size_t buffer_size) const;
      virtual void setCS(char *cData);
      virtual void checkCS(const char *cData);
      virtual void parseReply(const std::vector<uint8_t>* buffer);
      std::deque<std::vector<uint8_t> > mMsgQueue;
    private:
      Config mConfig;
      raw::CMD mLastCmd;
      ActData mActData;
      
  };
}

#endif