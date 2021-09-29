//
//  TempSensor.hpp
//  pumphouse
//
//  Created by Vincent Moscaritolo on 9/11/21.
//

#ifndef TempSensor_hpp
#define TempSensor_hpp
 
#include <stdio.h>
#include <functional>
#include <string>
#include <sys/time.h>

#include "SerialStream.hpp"
#include <map>
#include "PumpHouseDevice.h"
#include "TMP102.hpp"

using namespace std;

class TempSensor : public PumpHouseDevice{
 
public:
	TempSensor();
	~TempSensor();

	bool begin(int deviceAddress, string resultKey,  int *error = NULL);
	void stop();

	bool isConnected();
 
	response_result_t rcvResponse(std::function<void(map<string,string>)> callback = NULL);
	
	void idle(); 	// called from loop
	void reset(); 	// reset from timeout

	device_state_t getDeviceState();
	
private:

	typedef enum  {
		INS_UNKNOWN = 0,
		INS_IDLE ,
		INS_INVALID,
		INS_RESPONSE,
	 
	}in_state_t;

	
	in_state_t 		_state;
	map<string,string> _resultMap;
 
	TMP102				_sensor;
	string			_resultKey;
	
	timeval			_lastQueryTime;
	uint64_t     	_queryDelay;			// how long to wait before next query

};
#endif /* TempSensor_hpp */
