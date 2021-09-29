//
//  SmartShunt.hpp
//  pumphouse
//
//  Created by Vincent Moscaritolo on 9/7/21.
//

#ifndef SmartShunt_hpp
#define SmartShunt_hpp

#include <stdio.h>
#include "SerialStream.hpp"
#include <string>
#include <functional>
#include <vector>

#include <map>
#include <utility>

#include <time.h>

#include "PumpHouseDevice.h"

using namespace std;

class SmartShunt : public PumpHouseDevice{

public:
 
	typedef enum {
		INVALID = 0,
		PROCESS_VALUES,
		ERROR,
		FAIL,
		CONTINUE,
		NOTHING,
	}smartshunt_result_t;

	SmartShunt();
	~SmartShunt();

	bool begin(string path, int *error = NULL);
	void stop();

	bool isConnected();

	PumpHouseDevice::response_result_t rcvResponse(std::function<void(map<string,string>)> callback = NULL);
	
	void idle(); 	// called from loop
	
	void reset(); 	// reset from timeout

	int getFD() {return _stream.getFD();};
	
	device_state_t getDeviceState();
	
private:

	void  dumpMap();
	map<string,string> getValues() {return _resultMap;}
	 
	PumpHouseDevice::response_result_t process_char( uint8_t ch);
	SerialStream _stream;
	 
	map<string,vector<pair<time_t, string>>> _values;
 
	void processResults();

	  typedef enum  {
		  INS_INVALID = 0,
		  INS_IDLE ,
		  INS_RECORD_BEGIN,
		  INS_RECORD_NAME,
		  INS_RECORD_VALUE,
		  INS_CHECKSUM,
		  }in_state_t;
	in_state_t 	_state;

	
	// dynamic values.
	map<string,string> _resultMap;
	string		_rName;
	string 	_rValue;
	uint8_t 	_checkSum;
};

#endif /* SmartShunt_hpp */
