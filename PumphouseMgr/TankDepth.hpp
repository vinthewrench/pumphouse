//
//  TankDepth.hpp
//  pumphouse
//
//  Created by Vincent Moscaritolo on 9/12/21.
//

#ifndef TankDepth_hpp
#define TankDepth_hpp
 
#include <stdio.h>
#include <functional>
#include <string>
#include <sys/time.h>

#include "SerialStream.hpp"
#include <map>
#include "PumpHouseDevice.h"
#include "PumpHouseDB.hpp"

#include "MCP3427.hpp"

using namespace std;

class TankDepth : public PumpHouseDevice{
 
public:
	TankDepth(PumpHouseDB *db);
	~TankDepth();
 
	bool begin(int deviceAddress, string resultKey, string rawResultKey = "", int *error = NULL);
	void stop();

	bool isConnected();
 
	response_result_t rcvResponse(std::function<void(map<string,string>)> callback = NULL);
	
	void idle(); 	// called from loop

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
 
	MCP3427			_sensor;
	string				_resultKey;
	string				_rawResultKey;
 
	uint16_t 			_valFull;			// upper range value	- Tank full
	uint16_t 			_valEmpty;			// lower range value	- Tank empty
	
	timeval			_lastQueryTime;
	uint64_t     	_queryDelay;			// how long to wait before next query

	PumpHouseDB*	_db;
};
#endif /* TankDepth_hpp */
