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

using namespace std;

constexpr string_view INVERTER_IP_VOLTS		 			= "I_IPV";
constexpr string_view INVERTER_IP_FAULT_VOLTS		 	= "I_IPFV";
constexpr string_view INVERTER_OP_VOLTS		 			= "I_OPV";
constexpr string_view INVERTER_OP_CURRENT	 			= "I_OPC";
constexpr string_view INVERTER_OP_FREQ	 				= "I_FREQ";
constexpr string_view INVERTER_BATTERY_V	 				= "I_BV";
constexpr string_view INVERTER_BATTERY_TEMP 			= "I_BT";
constexpr string_view INVERTER_UPS_STATUS 				= "I_STATUS";


class SigineerInverter {

public:


	typedef enum {
		INVALID = 0,
		PROCESS_VALUES,
		ERROR,
		FAIL,
		CONTINUE,
		NOTHING,
	}inverter_result_t;

	SigineerInverter();
	~SigineerInverter();

	bool begin(const char * path, int *error = NULL);
	void stop();

	bool isConnected();
 
	inverter_result_t rcvResponse(std::function<void(map<string,string>)> callback = NULL);
	
	void idle(); 	// called from loop

	int getFD() {return _stream.getFD();};
 	
private:

	typedef enum  {
		INS_UNKNOWN = 0,
		INS_IDLE ,
		INS_SENT_QUERY,
		INS_RESPONSE,
 		INS_INVALID,
	 
	}in_state_t;
 
	in_state_t 	_state;
	string		_response;

	map<string,string> _resultMap;
	  
	inverter_result_t process_char( uint8_t ch);
	SerialStream _stream;
	
	timeval			_lastQueryTime;
	uint64_t     	_queryDelay;			// how long to wait before next query
  
	
};
#endif /* SigineerInverter_hpp */
