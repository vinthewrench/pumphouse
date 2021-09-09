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

using namespace std;

class SmartShunt {

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

	bool begin(const char * path, int *error = NULL);
	void stop();

	bool isConnected();

	smartshunt_result_t rcvResponse(std::function<void(map<string,string>)> callback = NULL);
	
	void idle(); 	// called from loop
	
	int getFD() {return _stream.getFD();};
	
private:

	void  dumpMap();
	map<string,string> getValues() {return _resultMap;}
	
	
 
	smartshunt_result_t process_char( uint8_t ch);
	SerialStream _stream;
	 
	map<string,vector<pair<time_t, string>>> _values;
 
	void processResults();

	  typedef enum  {
		  INS_UNKNOWN = 0,
		  INS_IDLE ,
		  INS_RECORD_BEGIN,
		  INS_RECORD_NAME,
		  INS_RECORD_VALUE,
		  INS_CHECKSUM,
		  INS_INVALID,
		
	  }in_state_t;
	in_state_t 	_state;

	
	// dynamic values.
	map<string,string> _resultMap;
	string		_rName;
	string 	_rValue;
	uint8_t 	_checkSum;
};

#endif /* SmartShunt_hpp */
