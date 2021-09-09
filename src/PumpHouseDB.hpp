//
//  PumpHouseDB.hpp
//  pumphouse
//
//  Created by Vincent Moscaritolo on 9/8/21.
//

#ifndef PumpHouseDB_hpp
#define PumpHouseDB_hpp


#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <mutex>
#include <random>
#include <set>
#include <vector>
#include <tuple>
#include <map>
#include <vector>
#include <string>


 using namespace std;

class PumpHouseDB {
	
	typedef enum {
		INVALID = 0,
		BOOL,				// Bool ON/OFF
		INT,				// Int
		MAH,				// mAh milliAmp hours
		PERMILLE, 		// (per thousand) sign ‰
		DEKAWATTHOUR,	 // .01kWh
		WATTS, 			// W
		MILLIVOLTS,		// mV
		MILLIAMPS,		// mA
		SECONDS,			// sec
		MINUTES,			// mins
		DEGREES_C,		// degC
		VOLTS,			// V
		HERTZ,			// Hz
		AMPS,				// A
		BINARY,			// Binary 8 bits 000001
		VE_PRODUCT,		// VE.PART
		STRING,			// string
		IGNORE,
		UNKNOWN,
	}valueSchemaUnits_t;
	
	typedef enum {
		TR_IGNORE 	= 0,
		TR_STATIC	= 1,
		TR_TRACK		= 2,
		TR_DONT_TRACK =  3 // use latest value, dont track
	}valueTracking_t;
	

 	typedef struct {
		string  					description;
		valueSchemaUnits_t  	units;
		valueTracking_t			tracking;
 	} valueSchema_t;

public:
 
	PumpHouseDB();
  ~PumpHouseDB();

	void clearAll();
	
	bool insertValue(string key, string value, time_t when = 0);
	bool insertValues(map<string,string>  values, time_t when = 0);

	bool initValueInfoFromFile(string filePath);

	string displayStringForValue(string key, string value);
	void dumpMap();
	
private:
	
	mutable std::mutex _mutex;

	valueSchema_t schemaForKey(string key);
	valueSchemaUnits_t unitsForKey(string key);
	string   unitSuffixForKey(string key);
	
	double 	normalizedDoubleForValue(string key, string value);
	int 		intForValue(string key, string value);
	
	bool valueShouldUpdate(string key, string value);

	map<string,vector<pair<time_t, string>>> _values;
 
	map<string,valueSchemaUnits_t>  _schemaMap;
	map<string, valueSchema_t>	_schema;
	
};

#endif /* PumpHouseDB_hpp */
