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
#include<functional>
#include <map>
#include <string>
#include "json.hpp"

#include "PumphouseCommon.hpp"

using namespace std;
using namespace nlohmann;

typedef  unsigned long eTag_t;


class PumpHouseDB {
 
public:

	constexpr static string_view PROP_INVERTER_PORT			= "inverter-path";
	constexpr static string_view PROP_SMARTSHUNT_PORT			= "smartshunt-path";

	constexpr static string_view PROP_TANK_FULL 				= "prop-tank-full";
	constexpr static string_view PROP_TANK_EMPTY 				= "prop-tank-empty";
	constexpr static string_view PROP_TRIGGER_PREFIX 			= "trigger-";

	constexpr static string_view PROP_LOG_FLAGS						= "log-flags";
	constexpr static string_view PROP_LOG_FILE						= "log-filepath";
	constexpr static string_view PROP_API_KEY						= "api-key";
	constexpr static string_view PROP_API_SECRET					= "api-secret";

	typedef enum {
		INVALID = 0,
		BOOL,				// Bool ON/OFF
		INT,				// Int
		MAH,				// mAh milliAmp hours
		PERMILLE, 		// (per thousand) sign ‰
		PERCENT, 		// (per hundred) sign ‰
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
	
private:
	
	typedef struct {
	  string  					description;
	  valueSchemaUnits_t  		units;
	  valueTracking_t			tracking;
  } valueSchema_t;

public:
	constexpr static string_view JSON_ARG_NAME			= "name";
 	constexpr static string_view JSON_ARG_TRACKING		= "tracking";
 	constexpr static string_view JSON_ARG_UNITS			= "units";
	constexpr static string_view JSON_ARG_SUFFIX			= "suffix";
	constexpr static string_view JSON_ARG_VALUE			= "value";
	constexpr static string_view JSON_ARG_TIME			= "time";
	constexpr static string_view JSON_ARG_DISPLAYSTR		= "display";
 
	PumpHouseDB();
  ~PumpHouseDB();
	bool initSchemaFromFile(string filePath);

	void clear();
	
	bool insertValue(string key, string value, time_t when,  eTag_t eTag);
	bool insertValues(map<string,string>  values, time_t when = 0);

	void addSchema(string key,  valueSchemaUnits_t units, string description, valueTracking_t tracking);

	vector<string> keysChangedSinceEtag( eTag_t eTag);

	string displayStringForValue(string key, string value);
	void dumpMap();

	// MARK: -  API Secrets
	bool apiSecretCreate(string APIkey, string APISecret);
	bool apiSecretDelete(string APIkey);
	bool apiSecretSetSecret(string APIkey, string APISecret);
	bool apiSecretGetSecret(string APIkey, string &APISecret);
	bool apiSecretMustAuthenticate();

	// MARK: -   SERVER PORTS
	void  setAllowRemoteTelnet(bool remoteTelnet);
	bool  getAllowRemoteTelnet();

	void  setTelnetPort(int port);
	int  	getTelnetPort();
	void  setRESTPort(int port);
	int  	getRESTPort();

	json	schemaJSON();
	json	currentValuesJSON(eTag_t  eTag = 0);
	json  jsonForValue(string key, string value);
	eTag_t lastEtag() { return  _eTag;};

	// MARK: - properties
	
	bool savePropertiesToFile(string filePath = "") ;
	bool restorePropertiesFromFile(string filePath = "");
 
	bool setProperty(string key, string value);
	bool getProperty(string key, string *value);
	bool setPropertyIfNone(string key, string value);

	bool getUint16Property(string key, uint16_t * value);
	bool removeProperty(string key);
	map<string ,string> getProperties();

private:
	
	mutable std::mutex _mutex;

	string defaultPropertyFilePath();
	
	valueSchema_t schemaForKey(string key);
	valueSchemaUnits_t unitsForKey(string key);
	string   unitSuffixForKey(string key);
	
	double 	normalizedDoubleForValue(string key, string value);
	int 		intForValue(string key, string value);
	
	bool valueShouldUpdate(string key, string value);

	map<string,vector<pair<time_t, string>>> _values;
 
	map<string,valueSchemaUnits_t>  _schemaMap;
	map<string, valueSchema_t>	_schema;
	map<string, eTag_t> 			_etagMap;
	eTag_t							_eTag;		// last change tag

	map<string,string> 			_properties;
	string 						_propertyFilePath;

};

#endif /* PumpHouseDB_hpp */
