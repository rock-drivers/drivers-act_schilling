#include "Driver.hpp"
#include <iostream>
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
//                    cout << "read: try readPacket" << endl;
        int size = readPacket(&buffer[0], buffer.size());
        //            cout << "read: readPacket: " << size << endl;
 
	if(size){
	  parseReply(&buffer);
	}
    } catch ( std::runtime_error &e) {
        //            cerr << "exception caught: " << e.what() << endl;
        throw;
    }
}

void Driver::writeNext()
{
    if (!mMsgQueue.empty()) {
        std::vector<uint8_t> msg = mMsgQueue.front();
        mMsgQueue.pop_front();
	mLastCmd = (act_schilling::raw::CMD)(((act_schilling::raw::MsgHeader*)msg.data())->cmd);
	writePacket(msg.data(), msg.size());
	cout <<"writing " <<msg[2] <<endl;
	read();
    }
}


