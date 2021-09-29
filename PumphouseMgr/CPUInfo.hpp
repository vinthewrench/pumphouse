//
//  CPUInfo.hpp
//  pumphouse
//
//  Created by Vincent Moscaritolo on 9/14/21.
//

#ifndef CPUInfo_hpp
#define CPUInfo_hpp

#include <stdio.h>
#include <functional>
#include <string>
#include <sys/time.h>

#include "SerialStream.hpp"
#include <map>
#include "PumpHouseDevice.h"

using namespace std;


constexpr string_view CPU_INFO_TEMP	 	= "CPU_TEMP";

class CPUInfo : public PumpHouseDevice{
 
public:
	CPUInfo();
	~CPUInfo();

	bool begin(int *error = NULL);
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

	
	in_state_t 			_state;
 	map<string,string> _resultMap;
 
	timeval			_lastQueryTime;
	uint64_t     	_queryDelay;			// how long to wait before next query

	
	bool getCPUTemp(double* tempOut);

};
#endif /* CPUInfo_hpp */
 
 
