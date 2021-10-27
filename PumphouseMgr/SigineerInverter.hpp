//
//  SigineerInverter.hpp
//  pumphouse
//
//  Created by Vincent Moscaritolo on 9/7/21.
//

#ifndef SigineerInverter_hpp
#define SigineerInverter_hpp

#include <stdio.h>
#include <functional>
#include <string>
#include <sys/time.h>

#include "SerialStream.hpp"
#include <map>
#include "PumpHouseDevice.h"
#include "PumpHouseDB.hpp"


using namespace std;

constexpr string_view INVERTER_IP_VOLTS		 			= "I_IPV";
constexpr string_view INVERTER_IP_FAULT_VOLTS		 	= "I_IPFV";
constexpr string_view INVERTER_OP_VOLTS		 			= "I_OPV";
constexpr string_view INVERTER_OP_CURRENT	 			= "I_OPC";
constexpr string_view INVERTER_OP_FREQ	 				= "I_FREQ";
constexpr string_view INVERTER_BATTERY_V	 				= "I_BV";
//constexpr string_view INVERTER_BATTERY_TEMP 			= "I_BT";
constexpr string_view INVERTER_UPS_STATUS 				= "I_STATUS";

class SigineerInverter : public PumpHouseDevice{
 
public:
	
	typedef enum {
		RESTARTED = 0,
		RESPONDING,
		TIMEOUT,
	}in_comm_t;

 	SigineerInverter(PumpHouseDB *db);
	~SigineerInverter();

	bool begin(string path, int *error = NULL);
	void stop();

	bool isConnected();
	time_t lastReponseTime() { return _lastResponseTime;}
	bool isResponding()			{return _isResponding;}
	bool isBypass()				{return _isBypass;}
	bool isFastCharge()			{return _isFastCharge;}
	bool isFloatCharge()		{return _isFloatCharge;}
 
	response_result_t rcvResponse(std::function<void(map<string,string>)> callback = NULL);
	
	void idle(); 	// called from loop

	int getFD() {return _stream.getFD();};
	device_state_t getDeviceState();

	void reset(); 	// reset from timeout
	
private:

	void eventProcessor();
	void sendQuery();
	
	typedef enum  {
		INS_UNKNOWN = 0,
		INS_IDLE ,
		INS_SENT_QUERY,
		INS_RESPONSE,
 		INS_INVALID,
		INS_TIMEOUT
	}in_state_t;

	typedef enum  {
		INV_UNKNOWN  = 0,
		INV_NO_RESPONSE,
		INV_FAIL,
		INV_BYPASS,
		INV_INVERTER,
		INV_FAST_CHARGE,
		INV_FLOAT_CHARGE,
//		INV_BAT_LOW
	}inverterState_t;

	in_state_t 		_state;			// input processor state
	inverterState_t	_inv_state;		// inverter processor state
	string				_response;
	
	time_t		_lastResponseTime;
	bool			_isResponding;
	bool			_isInverterOn;		// inverter on Inverter
	bool			_isBypass;			// inverter on Bypass
	bool			_isFastCharge;		// inverter charging above 29v
	bool			_isFloatCharge;	// inverter float charge
	
	map<string,string> _resultMap;
	  
	PumpHouseDevice::response_result_t process_char( uint8_t ch);
	SerialStream _stream;
	
	timeval			_lastQueryTime;
	uint64_t     	_queryDelay;			// how long to wait before next query
	uint64_t     	_timeoutDelay;			 

	uint8_t	     	_maxRetryCount;		// max amount of timeouts
	uint8_t	     	_retryCount;

	float				_batVoltFloat;
	float				_batVoltFast;

	PumpHouseDB*	_db;

};
#endif /* SigineerInverter_hpp */
