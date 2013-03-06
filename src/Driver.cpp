#include "Driver.hpp"
#include <iostream>
#include <string.h>
#include <base_schilling/SchillingRaw.hpp>

using namespace act_schilling;
using namespace std;

Driver::Driver(const Config& config)
    : ActHandler(config)
{
}

void Driver::read()
{
    std::vector<uint8_t>    buffer(1024);

    try {
                    //cout << "read: try readPacket" << endl;
        int size = readPacket(&buffer[0], buffer.size());
                    //cout << "read: readPacket: " << size << endl;
 
	if(size){
	  /*char sz[128];
	  *sz = 0;
	  for(int i=0;i<size;i++){
	    sprintf(sz+strlen(sz),"%02x | ",buffer[i]);
	  }    
	  cout <<"Actuator: read: " <<sz <<endl;*/
	  parseReply(&buffer);
	}
    } catch ( std::runtime_error &e) {
        cerr << "read: exception caught: " << e.what() << endl;
        throw;
    }
}

void Driver::writeNext()
{
    if (!mMsgQueue.empty()) {
        std::vector<uint8_t> msg = mMsgQueue.front();
        mMsgQueue.pop_front();
	mLastCmd = (act_schilling::raw::CMD)(((act_schilling::raw::MsgHeader*)msg.data())->cmd);
	/*char sz[128];
	*sz = 0;
	for(int i=0;i<msg.size();i++){
	  sprintf(sz+strlen(sz),"%02x | ",msg[i]);
	}	    
	cout <<"Actuator write: " <<sz <<endl;*/
    	writePacket(msg.data(), msg.size());
    }
}

void Driver::clearReadBuffer()
{
    std::vector<uint8_t>    buffer(1024);

    try {
      int size = readPacket(&buffer[0], buffer.size(),base::Time::fromSeconds(0.05));
      /*if(size){
	  char sz[128];
	  *sz = 0;
	  for(int i=0;i<size;i++){
	    sprintf(sz+strlen(sz),"%02x | ",buffer[i]);
	  }		    
	  cout <<"Actuator: discarded read: " <<sz <<endl;
      }*/
    } catch ( std::runtime_error &e) {
	cerr << "read: discarded exception caught: " << e.what() << endl;
    }
}

