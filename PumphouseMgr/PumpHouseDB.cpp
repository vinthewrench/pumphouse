//
//  PumpHouseDB.cpp
//  pumphouse
//
//  Created by Vincent Moscaritolo on 9/8/21.
//

#include "PumpHouseDB.hpp"
#include "TimeStamp.hpp"
#include "LogMgr.hpp"
#include "Utils.hpp"
#include <stdlib.h>
#include <regex>

PumpHouseDB::PumpHouseDB(){
	
	_values.clear();
	_schema.clear();
	
	_schemaMap = {
		{"Bool", BOOL},				// Bool ON/OFF
		{"Int", INT},				// Int
		{"mAh", MAH},				// mAh milliAmp hours
		{"‰", PERMILLE} ,			// (per thousand) sign ‰
		{".01kWh", DEKAWATTHOUR},		// .01kWh
		{"W", WATTS}, 				// W
		{"mA",MILLIAMPS},			// mA
		{"mV", MILLIVOLTS},			// mV
		{"sec", SECONDS},			// sec
		{"mins",MINUTES},			// mins
		{"degC", DEGREES_C},		// degC
		{"V", VOLTS},				// V
		{"Hz", HERTZ},				// Hz
		{"A", AMPS},					// A
		{"Binary", BINARY},			// Binary 8 bits 000001
		{"VE.PART", VE_PRODUCT},		// VE.PART
		{"string", STRING},				// string
		{"ignore", IGNORE}				// ignore
	};
 }

PumpHouseDB::~PumpHouseDB(){
 
}


void PumpHouseDB::clear(){
	_values.clear();

}

bool PumpHouseDB::insertValues(map<string,string>  values, time_t when){
	
	bool didUpdate = false;
	if(when == 0)
		when = time(NULL);
	
	for (auto& [key, value] : values) {
		if(insertValue(key, value, when)){
			didUpdate = true;
		}
	}
	
	return didUpdate;
	
};

bool PumpHouseDB::insertValue(string key, string value, time_t when){
	
	bool updated = false;
	
	if(when == 0)
		when = time(NULL);
	
	valueSchema_t schema = schemaForKey(key);
	if(schema.tracking == TR_IGNORE){
		updated = false;
	}
	else if(schema.tracking == TR_DONT_TRACK)
	{
		updated = valueShouldUpdate(key,value);
	// only keep last value
		_values[key] = {make_pair(when, value)};
 	}
	else if(valueShouldUpdate(key,value)){
		_values[key].push_back(make_pair(when, value));
		updated = true;
	}
	
	///////// debug
	if(updated){
		string str =  displayStringForValue(key, value);
		printf("%3lu %-10s : %-16s %s\n",
				 _values[key].size(),
				 key.c_str(),
				 str.c_str(),
				 schema.description.c_str()
				 );
	}
	///////// debug

	return updated;
}


bool PumpHouseDB::valueShouldUpdate(string key, string value){
	
	bool shouldInsert = true;
	
	valueSchema_t schema = schemaForKey(key);
	if(schema.tracking == TR_IGNORE)
		return false;
	
	if(_values.count(key)){
		auto lastpair = _values[key].back();
		valueSchema_t vs = _schema[key];
		
		// do we ignore it
		if(vs.units == IGNORE)
			return false;
		
		// quick string compare to see if its the same
		if(lastpair.second == value)
 			return false;
		
		// see if it's a number
		char *p1, *p;
		double newVal = strtod(value.c_str(), &p);
		double oldval = strtod(lastpair.second.c_str(), &p1);
		if(*p == 0 && *p1 == 0 ){
			
			double diff = abs(oldval-newVal);
			
			switch (vs.units) {
				case MILLIVOLTS:
				case MILLIAMPS:
				case MAH:
					if(diff < 500)
						shouldInsert = false;
					//				printf(" %f -  %f = %f %s\n", oldval, newVal, diff, shouldInsert?"T":"F" );
					break;
			
	
				case WATTS:
				case VOLTS:
					if(diff < 5)
						shouldInsert = false;
					//		 				printf(" %f -  %f = %f %s\n", oldval, newVal, diff, shouldInsert?"T":"F" );
					break;
					
				case PERMILLE:
					if(diff < 10)
						shouldInsert = false;
					break;
					
				case HERTZ:
					if(diff < 10)
						shouldInsert = false;
					//				printf(" %f -  %f = %f %s\n", oldval, newVal, diff, shouldInsert?"T":"F" );
					
				default:
					break;
			}
			
		}
	}
	
	return shouldInsert;
}


PumpHouseDB::valueSchema_t PumpHouseDB::schemaForKey(string key){
	
	valueSchema_t schema = {"",UNKNOWN,TR_IGNORE};
 
	if(_schema.count(key)){
		schema =  _schema[key];
	}
	
	return schema;
}

PumpHouseDB::valueSchemaUnits_t PumpHouseDB::unitsForKey(string key){
	valueSchema_t schema = schemaForKey(key);
	return schema.units;
}

string   PumpHouseDB::unitSuffixForKey(string key){
	string suffix = {};
	
 	switch(unitsForKey(key)){
			
		case MILLIVOLTS:
		case VOLTS:
			suffix = "V";
			break;

		case MILLIAMPS:
		case AMPS:
			suffix = "A";
			break;

		case MAH:
			suffix = "Ahrs";
			break;
 
		case DEKAWATTHOUR:
			suffix = "kWh";
			break;
  
		case DEGREES_C:
			suffix = "ºC";
			break;
 
		case PERMILLE:
			suffix = "%";
			break;

		case WATTS:
			suffix = "W";
			break;

		case SECONDS:
 			suffix = "Seconds";
			break;
			
		case MINUTES:
			suffix = "Minutes";
			break;

		case HERTZ:
			suffix = "Hz";
			break;
 
		default:
			break;
	}
	
	return suffix;
}

double PumpHouseDB::normalizedDoubleForValue(string key, string value){
	
	double retVal = 0;
	
	// see if it's a number
	char   *p;
	double val = strtod(value.c_str(), &p);
	if(*p == 0) {
 
		// normalize number
		
		switch(unitsForKey(key)){
				
			case MILLIVOLTS:
			case MILLIAMPS:
			case MAH:
				retVal = val / 1000;
				break;
				
			case DEKAWATTHOUR:
			case DEGREES_C:
				retVal = val / 100;
				break;
	
			case PERMILLE:
				retVal = val / 10;
				break;

			case WATTS:
			case VOLTS:
			case AMPS:
			case SECONDS:
			case MINUTES:
			case HERTZ:
				retVal = val;
				break;
				
			default:
				break;
		}
	}
	return retVal;
}
 int PumpHouseDB::intForValue(string key, string value){
	
	int retVal = 0;
	
	 switch(unitsForKey(key)){

		 case MINUTES:
		 case SECONDS:
		 case INT:
		 {
			 int intval = 0;

			 if(sscanf(value.c_str(), "%d", &intval) == 1){
				retVal = intval;
			}
		 }
			 break;
			 
		 case VE_PRODUCT:
		 {
			 uint16_t int16 = 0;
		 
			 if( regex_match(value, std::regex("^0?[xX][0-9a-fA-F]{4}$"))
						&& ( std::sscanf(value.c_str(), "%hx", &int16) == 1)){
				 retVal = int16;
			 }

		 }
			 break;
			 
			 default:
			 break;
	 }
	  
	return retVal;
}



string PumpHouseDB::displayStringForValue(string key, string value){
	
	string  retVal = value;

	string suffix =  unitSuffixForKey(key);

	switch(unitsForKey(key)){

		case MILLIVOLTS:
		case MILLIAMPS:
		case MAH:
		case VOLTS:
		case AMPS:
		case HERTZ:
		case WATTS:
		case DEKAWATTHOUR:
		case PERMILLE:
		case DEGREES_C:
		{
			double val = normalizedDoubleForValue(key, value);
			char buffer[12];
			sprintf(buffer, "%3.2f%s", val, suffix.c_str());
			retVal = string(buffer);
		}
			break;

		case SECONDS:
		case MINUTES:
		{
			int val  = intForValue(key, value);
			if(val == -1){
				retVal = "Infinite";
			}
			else
			{
				char buffer[64];
				sprintf(buffer, "%d %s", val, suffix.c_str());
				retVal = string(buffer);
			}
		}
			break;

		case VE_PRODUCT:
		{
			int val = intForValue(key, value);
			switch(val){
				case 0xA389:
					retVal = "SmartShunt 500A/50mV";
					break;

				default:
					break;
			}
		}
			
		default:
		break;
	}
	
	return retVal;
}


void PumpHouseDB::dumpMap(){
	
	timestamp::TimeStamp ts;
	
	printf("\n -- %s --\n", ts.logFileString().c_str());
	
	for (auto& [key, value] : _values) {
		
		auto lastpair = _values[key].back();
		auto count = _values.count(key);
		
		string desc = "";
		if(_schema.count(key)){
			desc = _schema[key].description;
		}
		
		printf("%3lu %-8s:%10s %s\n",
				 count,
				 key.c_str(),
				 lastpair.second.c_str(),
				 desc.c_str()
				 );
	}
}


 
bool PumpHouseDB::initValueInfoFromFile(string filePath){
	bool 				statusOk = false;
	
	std::ifstream	ifs;
	
	// create a file path
	if(filePath.size() == 0)
		filePath = "valueschema.csv";
 
	LOG_INFO("READ valueschema: %s\n", filePath.c_str());
	
	try{
		string line;
		std::lock_guard<std::mutex> lock(_mutex);
	
		_schema.clear();

		// open the file
		ifs.open(filePath, ios::in);
		if(!ifs.is_open()) return false;
	
		while ( std::getline(ifs, line) ) {
			
			// split the line looking for a token: and rest and ignore comments
			line = Utils::trimStart(line);
			if(line.size() == 0) continue;
			if(line[0] == '#')  continue;
			
			vector<string> v = split<string>(line, ",");
			if(v.size() != 4)  continue;
			
			string key = v[0];
			string typ = v[1];
			string track = v[2];
			string desc = v[3];
			
			if(_schemaMap.count(typ)){
				valueSchema_t sch ;
				sch.units =  _schemaMap[typ];
				sch.description = desc;
				sch.tracking = (valueTracking_t) std::stoi( track );
				_schema[key] = sch;
				
			}
		}
		
		statusOk = _schema.size() > 0;
		ifs.close();
	}
	catch(std::ifstream::failure &err) {
		
		LOG_INFO("READ valueschema:FAIL: %s\n", err.what());
		statusOk = false;
	}
	
	return statusOk;
	
}



// MARK: -  API Secrets
bool PumpHouseDB::apiSecretCreate(string APIkey, string APISecret){
	return true;
}

bool PumpHouseDB::apiSecretSetSecret(string APIkey, string APISecret){
	return true;
}

bool PumpHouseDB::apiSecretDelete(string APIkey){
	return true;

}

bool PumpHouseDB::apiSecretGetSecret(string APIkey, string &APISecret){
	return false;
}

bool PumpHouseDB::apiSecretMustAuthenticate(){
	return false;
}

// MARK: -   SERVER PORTS
void  PumpHouseDB::setAllowRemoteTelnet(bool remoteTelnet) {
};

bool  PumpHouseDB::getAllowRemoteTelnet() {
	return true;
};

void  PumpHouseDB::setTelnetPort(int port){
}

int  	PumpHouseDB::getTelnetPort(){
	return 2020;
}

void  PumpHouseDB::setRESTPort(int port){
}

int PumpHouseDB::getRESTPort(){
	return 8080;
}

// MARK: -   JSON REQUESTS


json PumpHouseDB::schemaJSON(){
	
	json schemaList;
 
	for (auto& [key, sch] : _schema) {
 
		json entry;
		entry[string(JSON_ARG_NAME)] =   sch.description;
		entry[string(JSON_ARG_UNITS)] =   sch.units;
		entry[string(JSON_ARG_TRACKING)] =   sch.tracking;
		entry[string(JSON_ARG_SUFFIX)] =   unitSuffixForKey(key);
		schemaList[key] = entry;
	}
	
	return schemaList;
}

json PumpHouseDB::currentValuesJSON(){
	json j;

	timestamp::TimeStamp ts;

	for (auto& [key, value] : _values) {
	
		auto lastpair = _values[key].back();
	
		time_t t = lastpair.first;
	 
		json entry;
		entry[string(JSON_ARG_VALUE)] 		=   jsonForValue(key, lastpair.second);
		entry[string(JSON_ARG_DISPLAYSTR)] =   displayStringForValue(key, lastpair.second);
		entry[string(JSON_ARG_TIME)] 		=   t;
 		j[key] = entry;
	}
	
	return j;

 }
	
json PumpHouseDB::jsonForValue(string key, string value){
	json j;

	string suffix =  unitSuffixForKey(key);

	switch(unitsForKey(key)){

		case MILLIVOLTS:
		case MILLIAMPS:
		case MAH:
		case VOLTS:
		case AMPS:
		case HERTZ:
		case WATTS:
		case DEKAWATTHOUR:
		case PERMILLE:
		case DEGREES_C:
		{
			double val = normalizedDoubleForValue(key, value);
			j = val;
		}
			break;

		case SECONDS:
		case MINUTES:
		{
			int val  = intForValue(key, value);
			j = val;
		}
			break;
			
 		case VE_PRODUCT:
		{
			int val = intForValue(key, value);
			j = val;
		}
			break;

		default:
			j = value;
		break;
	}
	return j;
}

