//
//  PumpHouseDevice.h
//  pumphouse
//
//  Created by Vincent Moscaritolo on 9/10/21.
//

#ifndef PumpHouseDevice_h
#define PumpHouseDevice_h

#include <string>
#include <map>

#include "PumphouseCommon.hpp"

class PumpHouseDevice {
	
public:
	
	typedef enum  {
		DEVICE_STATE_UNKNOWN = 0,
		DEVICE_STATE_DISCONNECTED,
		DEVICE_STATE_CONNECTED,
		DEVICE_STATE_ERROR
	}device_state_t;
	
	typedef enum {
		INVALID = 0,
		PROCESS_VALUES,
		ERROR,
		FAIL,
		CONTINUE,
		NOTHING,
	}response_result_t;
	
	virtual bool begin(const char * path, int *error = NULL) = 0;
	virtual void stop() = 0;
	virtual bool isConnected()  = 0;
	virtual void idle() = 0; 	// called from loop
	virtual device_state_t getDeviceState() = 0;
	
	virtual PumpHouseDevice::response_result_t
	rcvResponse(std::function<void(std::map<std::string, std::string>)> callback = NULL) { return INVALID;};
	
};

#endif /* PumpHouseDevice_h */
