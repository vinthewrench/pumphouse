//
//  PumpHouseDB.cpp
//  pumphouse
//
//  Created by Vincent Moscaritolo on 9/8/21.
//

#include "PumpHouseDB.hpp"

#include "SigineerInverter.hpp"


#include "TimeStamp.hpp"
#include "LogMgr.hpp"
#include "Utils.hpp"
#include <stdlib.h>
#include <regex>

using namespace timestamp;

PumpHouseDB::PumpHouseDB(){
	
	_values.clear();
	_schema.clear();
	_eTag = 1;
	_etagMap.clear();
	_properties.clear();
	_sdb = NULL;
 	
	_schemaMap = {
		{"Bool", BOOL},				// Bool ON/OFF
		{"Int", INT},				// Int
		{"mAh", MAH},				// mAh milliAmp hours
		{"‰", PERMILLE} ,			// (per thousand) sign ‰
		{"%", PERCENT} ,			// (per hundred) sign PERCENT
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
	
	if(_sdb)
	{
		sqlite3_close(_sdb);
		_sdb = NULL;
	}
}


void PumpHouseDB::clear(){
	_values.clear();

}


bool PumpHouseDB::insertValues(map<string,string>  values, time_t when){
	
	bool didUpdate = false;
	if(when == 0)
		when = time(NULL);
	
	for (auto& [key, value] : values) {
		if(insertValue(key, value, when, _eTag )){
			didUpdate = true;
		}
	}
	
	if(didUpdate)
		_eTag++;
	
	return didUpdate;
	
};

bool PumpHouseDB::insertValue(string key, string value, time_t when, eTag_t eTag){

	bool updated = false;

	if(when == 0)
		when = time(NULL);
	
	valueSchema_t schema = schemaForKey(key);
	if(schema.tracking == TR_IGNORE){
		return false;
	}
	else if(schema.tracking == TR_STATIC) {
		//  keep last value
			_values[key] = make_pair(when, value) ;
		//  but dont put this in the database
	}
	else if(schema.tracking == TR_DONT_TRACK)
	{
	// only keep last value
		_values[key] = make_pair(when, value) ;
		
		 //  Add these to DB - but dont insert.. just update.
		saveUniqueValueToDB(key, value, when);
  	}
	else if(schema.tracking == TR_TRACK)
	{
		if(valueShouldUpdate(key,value)){
			
			// update value
			_values[key] = make_pair(when, value);

			// record in DB
			insertValueToDB(key, value, when);
			updated = true;
			}
		}
	
	if(updated){
		_etagMap[key] = eTag;
	}

 //		printf("%s %s: %s \n", updated?"T":"F",  key.c_str(), value.c_str());

	return updated;
}


vector<string> PumpHouseDB::keysChangedSinceEtag( eTag_t tag){
	vector<string> changeList;
	changeList.clear();
	
	for (auto& [key, t] : _etagMap) {
		
		if(tag <= t){
			changeList.push_back(key);
		}
	}

	return changeList;
}

bool PumpHouseDB::valueShouldUpdate(string key, string value){
	
	bool shouldInsert = true;
	double triggerDiff = 0;


	valueSchema_t schema = schemaForKey(key);
	if(schema.tracking == TR_IGNORE)
		return false;

	if(_values.count(key)){
		auto lastpair = _values[key];
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
				case DEGREES_C:
					triggerDiff = 1.0;
					break;
	
				case MILLIVOLTS:
				case MILLIAMPS:
				case MAH:
 					triggerDiff = 500;
 					break;
	
				case WATTS:
					triggerDiff = 5;
					break;
					
				case VOLTS:
					if(key == INVERTER_BATTERY_V )		// low voltage
						triggerDiff = 0.0;
					else
						triggerDiff = 4.0;
					break;

				case AMPS:
					triggerDiff = 1.0;
					break;
					
				case PERMILLE:
					triggerDiff = 10;
					break;
	
				case PERCENT:
					triggerDiff = 1;
					break;
					
				case HERTZ:
					triggerDiff = 10;
					break;
					
				default:
					triggerDiff = 0;
					break;
			}
			
			// override trigger
			string str;
			if(getProperty( string(PumpHouseDB::PROP_TRIGGER_PREFIX)+key, &str)){
				double trigVal = strtod(str.c_str(), &p);
				if(*p == 0){
					triggerDiff  = trigVal;
				}
			}
 				
			shouldInsert = diff > triggerDiff;
			
 	//		if(key == INVERTER_BATTERY_V)
//  				printf("%s %8s %5.3f -  %5.3f = %f.3 > %f.3\n", shouldInsert?"T":"F", key.c_str(),
// 					 oldval, newVal, diff , triggerDiff );
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
		case PERCENT:
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
				retVal = val / 100;
				break;
	
			case PERMILLE:
				retVal = val / 10;
				break;

			case PERCENT:
			case DEGREES_C:
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
		case PERCENT:
		{
			double val = normalizedDoubleForValue(key, value);
			char buffer[12];
			sprintf(buffer, "%3.2f%s", val, suffix.c_str());
			retVal = string(buffer);
		}
			break;

		case DEGREES_C:
		{
			double val = normalizedDoubleForValue(key, value);
			double tempF =  val * 9.0 / 5.0 + 32.0;
			
			char buffer[12];
			sprintf(buffer, "%3.2f%s", tempF, "°F");
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
		
		auto lastpair = _values[key];
		auto count = _values.count(key);
		
		string desc = "";
		if(_schema.count(key)){
			desc = _schema[key].description;
		}
		
		printf("%3d %-8s:%10s %s\n",
				 (int)count,
				 key.c_str(),
				 lastpair.second.c_str(),
				 desc.c_str()
				 );
	}
}
// MARK: - Events
bool PumpHouseDB::logEvent(ph_event_t evt, time_t when ){
	
	if(when == 0)
		when = time(NULL);

	auto ts = TimeStamp(when);

//printf("%s \t EVT: %s\n", ts.ISO8601String().c_str(), displayStringForEvent(evt).c_str());
			  
	string sql = string("INSERT INTO EVENT_LOG (EVENT,DATE) ")
			+ "VALUES  (" + to_string(evt) + ", '" + ts.ISO8601String() + "' );";
	
//  printf("%s\n", sql.c_str());
	
	char *zErrMsg = 0;
	if(sqlite3_exec(_sdb,sql.c_str(),NULL, 0, &zErrMsg  ) != SQLITE_OK){
		LOG_ERROR("sqlite3_exec FAILED: %s\n\t%s\n", sql.c_str(), sqlite3_errmsg(_sdb	) );
		sqlite3_free(zErrMsg);
		return false;
	}
	
	return true;
}

string PumpHouseDB::displayStringForEvent(ph_event_t evt){
	
	string result = "UNKNOWN";
	switch (evt) {

		case EV_START:
			result = "START";
			break;

		case EV_SHUTDOWN:
			result = "SHUTDOWN";
			break;

		case EV_INV_BYPASS:
			result = "BYPASS";
			break;

		case EV_INV_INVERTER:
			result = "INVERTER";
			break;

		case EV_INV_FLOAT:
			result = "FLOAT";
			break;

		case EV_INV_FAST_CHARGE:
			result = "FAST";
			break;

		case EV_INV_NOT_RESPONDING:
			result = "INV_NOT_RESPONDING";
			break;
			
		case EV_INV_FAIL:
			result = "INV_FAIL";
			break;

		default:;
	
	}
	
	return result;
}


// MARK: -  DATABASE OPERATIONS

bool PumpHouseDB::restoreValuesFromDB(){
	
	std::lock_guard<std::mutex> lock(_mutex);
	bool	statusOk = true;;
 
	_values.clear();
 
	string sql = string("SELECT NAME, VALUE, MAX(strftime('%s', DATE)) AS DATE FROM SENSOR_DATA GROUP BY NAME;");
	
	sqlite3_stmt* stmt = NULL;
	sqlite3_prepare_v2(_sdb, sql.c_str(), -1,  &stmt, NULL);

	while ( (sqlite3_step(stmt)) == SQLITE_ROW) {
	 
		string  key = string( (char*) sqlite3_column_text(stmt, 0));
		string  value = string((char*) sqlite3_column_text(stmt, 1));
		time_t  when =  sqlite3_column_int64(stmt, 2);
		
// 		printf("%8s  %8s %ld\n",  key.c_str(),  value.c_str(), when );
		_values[key] = make_pair(when, value) ;
		_etagMap[key] = _eTag++;
	}
	sqlite3_finalize(stmt);
	
		
	return statusOk;
}
 


bool PumpHouseDB::insertValueToDB(string key, string value, time_t time ){
	
	std::lock_guard<std::mutex> lock(_mutex);
	auto ts = TimeStamp(time);
	
	string sql = string("INSERT INTO SENSOR_DATA (NAME,DATE,VALUE) ")
			+ "VALUES  ('" + key + "', '" + ts.ISO8601String() + "', '" + value + "' );";
	
// printf("%s\n", sql.c_str());
	
	char *zErrMsg = 0;
	if(sqlite3_exec(_sdb,sql.c_str(),NULL, 0, &zErrMsg  ) != SQLITE_OK){
		LOG_ERROR("sqlite3_exec FAILED: %s\n\t%s\n", sql.c_str(), sqlite3_errmsg(_sdb	) );
		sqlite3_free(zErrMsg);
		return false;
	}
 
	return true;
}


bool PumpHouseDB::saveUniqueValueToDB(string key, string value, time_t time ){
	
	std::lock_guard<std::mutex> lock(_mutex);
	auto ts = TimeStamp(time);
	
	string sql =
	string("BEGIN;")
	+ string("DELETE FROM SENSOR_DATA WHERE NAME = '") + key + "';"
	+ string("INSERT INTO SENSOR_DATA (NAME,DATE,VALUE) ")
			+ "VALUES  ('" + key + "', '" + ts.ISO8601String() + "', '" + value + "' );"
	+ string("COMMIT;");

	/*
	 BEGIN;
	 DELETE FROM SENSOR_DATA  WHERE NAME = 'TANK_RAW';
	 INSERT INTO SENSOR_DATA (NAME,DATE,VALUE) VALUES  ('TANK_RAW', '2021-09-19 00:52:06 GMT', '1' );
	 COMMIT;

	 */
 //	printf("%s\n", sql.c_str());
	
	char *zErrMsg = 0;
	if(sqlite3_exec(_sdb,sql.c_str(),NULL, 0, &zErrMsg  ) != SQLITE_OK){
		LOG_ERROR("sqlite3_exec FAILED: %s\n\t%s\n", sql.c_str(), sqlite3_errmsg(_sdb	) );
		sqlite3_free(zErrMsg);
		return false;
	}
 
	return true;
}


bool PumpHouseDB::removeHistoryForKey(string key, float days){
	
	std::lock_guard<std::mutex> lock(_mutex);
	bool success = false;
	
	string sql = string("DELETE FROM SENSOR_DATA ");
	
	if(key.size() > 0) {
		sql += "WHERE NAME ='" + key + "' ";
	}
	
	if(days > 0) {
		
		if(key.size() > 0) {
			sql += "AND ";
		}
		else {
			sql += "WHERE ";
		}
		
		sql += "DATE < datetime('now' , '-" + to_string(days) + " days'); ";
	}
	else {
		sql += ";";
	}
	sqlite3_stmt* stmt = NULL;
	
	if(sqlite3_prepare_v2(_sdb, sql.c_str(), -1,  &stmt, NULL)  == SQLITE_OK){
		
		if(sqlite3_step(stmt) == SQLITE_DONE) {
 
			int count =  sqlite3_changes(_sdb);
			LOG_DEBUG("sqlite %s\n %d rows affected n", sql.c_str(), count );
			success = true;
		}
		else
		{
			LOG_ERROR("sqlite3_step FAILED: %s\n\t%s\n", sql.c_str(), sqlite3_errmsg(_sdb	) );
		}
		sqlite3_finalize(stmt);
		
	}
	else {
		LOG_ERROR("sqlite3_prepare_v2 FAILED: %s\n\t%s\n", sql.c_str(), sqlite3_errmsg(_sdb	) );
		sqlite3_errmsg(_sdb);
	}
	
	return success;
}

bool PumpHouseDB::historyForKey(string key, historicValues_t &valuesOut, float days, int limit){

	std::lock_guard<std::mutex> lock(_mutex);
	bool success = false;
	
	historicValues_t values;
	values.clear();

	string sql = string("SELECT strftime('%s', DATE) AS DATE, VALUE FROM SENSOR_DATA WHERE NAME = '")
	+ key + "'";
	
	if(limit){
		sql += " ORDER BY DATE DESC LIMIT " + to_string(limit) + ";";
	}
	else if(days > 0) {
		sql += " AND DATE > datetime('now' , '-" + to_string(days) + " days');";
	}
	else {
		sql += ";";
	}
	
	sqlite3_stmt* stmt = NULL;

 	sqlite3_prepare_v2(_sdb, sql.c_str(), -1,  &stmt, NULL);
	
	while ( (sqlite3_step(stmt)) == SQLITE_ROW) {
		time_t  when =  sqlite3_column_int64(stmt, 0);
		string  value = string((char*) sqlite3_column_text(stmt, 1));
		values.push_back(make_pair(when, value)) ;
	}
	sqlite3_finalize(stmt);

	success = values.size() > 0;
	
	if(success){
		valuesOut = values;
	}

	
	return success;
}

bool PumpHouseDB::historyForEvents( historicEvents_t &eventsOut, float days, int limit){
	std::lock_guard<std::mutex> lock(_mutex);
	bool success = false;
	
	historicEvents_t events;
	events.clear();

	string sql = string("SELECT strftime('%s', DATE) AS DATE, EVENT FROM EVENT_LOG ");
	 
	if(limit){
		sql += " ORDER BY DATE DESC LIMIT " + to_string(limit) + ";";
	}
	else if(days > 0) {
		sql += " WHERE DATE > datetime('now' , '-" + to_string(days) + " days');";
	}
	else {
		sql += ";";
	}
	
	sqlite3_stmt* stmt = NULL;

	sqlite3_prepare_v2(_sdb, sql.c_str(), -1,  &stmt, NULL);
	
	while ( (sqlite3_step(stmt)) == SQLITE_ROW) {
		time_t  when =  sqlite3_column_int64(stmt, 0);
		int  event =  sqlite3_column_int(stmt, 1);
		events.push_back(make_pair(when, (ph_event_t) event)) ;
	}
	sqlite3_finalize(stmt);

	success =  true ; //events.size() > 0;
	
	if(success){
		eventsOut = events;
	}
	
	return success;
	
}

bool PumpHouseDB::removeHistoryForEvents(float days){
	
	std::lock_guard<std::mutex> lock(_mutex);
	bool success = false;
	
	string sql = string("DELETE FROM EVENT_LOG ");
		
	if(days > 0) {
 		sql += "WHERE DATE < datetime('now' , '-" + to_string(days) + " days'); ";
	}
	else {
		sql += ";";
	}
	sqlite3_stmt* stmt = NULL;
	
	if(sqlite3_prepare_v2(_sdb, sql.c_str(), -1,  &stmt, NULL)  == SQLITE_OK){
		
		if(sqlite3_step(stmt) == SQLITE_DONE) {
 
			int count =  sqlite3_changes(_sdb);
			LOG_DEBUG("sqlite %s\n %d rows affected n", sql.c_str(), count );
			success = true;
		}
		else
		{
			LOG_ERROR("sqlite3_step FAILED: %s\n\t%s\n", sql.c_str(), sqlite3_errmsg(_sdb	) );
		}
		sqlite3_finalize(stmt);
		
	}
	else {
		LOG_ERROR("sqlite3_prepare_v2 FAILED: %s\n\t%s\n", sql.c_str(), sqlite3_errmsg(_sdb	) );
		sqlite3_errmsg(_sdb);
	}
	
	return success;
}



// MARK: -  SCHEMA


bool PumpHouseDB::initLogDatabase(string filePath){
	
	// create a file path
	if(filePath.size() == 0)
		filePath = "pumphouse.db";

	LOG_DEBUG("OPEN database: %s\n", filePath.c_str());

	//  Open database
	if(sqlite3_open(filePath.c_str(), &_sdb) != SQLITE_OK){
		LOG_ERROR("sqlite3_open FAILED: %s\n", filePath.c_str(), sqlite3_errmsg(_sdb	) );
		return false;
	}
	
	// make sure primary tables are there.
	string sql1 = "CREATE TABLE IF NOT EXISTS SENSOR_DATA("  \
						"NAME 			  TEXT     	NOT NULL," \
						"DATE          DATETIME	NOT NULL," \
						"VALUE         TEXT     	NOT NULL);";
	
	char *zErrMsg = 0;
	if(sqlite3_exec(_sdb,sql1.c_str(),NULL, 0, &zErrMsg  ) != SQLITE_OK){
		LOG_ERROR("sqlite3_exec FAILED: %s\n\t%s\n", sql1.c_str(), sqlite3_errmsg(_sdb	) );
		sqlite3_free(zErrMsg);
		return false;
	}
	// make sure primary tables are there.
	string sql2 = "CREATE TABLE IF NOT EXISTS EVENT_LOG("  \
						"EVENT 		  INTEGER    NOT NULL," \
						"DATE          DATETIME	NOT NULL);";
	
	if(sqlite3_exec(_sdb,sql2.c_str(),NULL, 0, &zErrMsg  ) != SQLITE_OK){
		LOG_ERROR("sqlite3_exec FAILED: %s\n\t%s\n", sql2.c_str(), sqlite3_errmsg(_sdb	) );
		sqlite3_free(zErrMsg);
		return false;
	}

	
	if(!restoreValuesFromDB()){
		LOG_ERROR("restoreValuesFromDB FAILED\n");
		return false;
	}
	
	return true;
}

bool PumpHouseDB::initSchemaFromFile(string filePath){
	bool	statusOk = false;
	
	std::ifstream	ifs;
	
	// create a file path
	if(filePath.size() == 0)
		filePath = "valueschema.csv";
 
	LOG_DEBUG("READ valueschema: %s\n", filePath.c_str());
	
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
				addSchema(key, _schemaMap[typ], desc, (valueTracking_t) std::stoi( track ));
			}
		}
		
		statusOk = _schema.size() > 0;
		ifs.close();
	}
	catch(std::ifstream::failure &err) {
		
		LOG_ERROR("READ valueschema:FAIL: %s\n", err.what());
		statusOk = false;
	}
	
	return statusOk;
	
}


void PumpHouseDB::addSchema(string key,  valueSchemaUnits_t units, string description, valueTracking_t tracking){
	
	valueSchema_t sc;
	sc.units = units;
	sc.description = description;
	sc.tracking = tracking;
	
	_schema[key] = sc;
}

// MARK: - properties
bool PumpHouseDB::setProperty(string key, string value){
	_properties[key] = value;
	savePropertiesToFile();
	return true;
}

bool PumpHouseDB::removeProperty(string key){
	
	if(_properties.count(key)){
		_properties.erase(key);
		savePropertiesToFile();
		return true;
	}
	return false;
}

bool PumpHouseDB::setPropertyIfNone(string key, string value){
	
	if(_properties.count(key) == 0){
		_properties[key] = value;
		savePropertiesToFile();
		return true;
	}
	return false;
}

map<string ,string> PumpHouseDB::getProperties(){
	
	return _properties;
}

bool PumpHouseDB::getProperty(string key, string *value){
	
	if(_properties.count(key)){
		if(value)
			*value = _properties[key];
		return true;
	}
	return false;
}

bool  PumpHouseDB::getUint16Property(string key, uint16_t * valOut){
	
	string str;
	if(getProperty(string(key), &str)){
		char* p;
		long val = strtoul(str.c_str(), &p, 0);
		if(*p == 0 && val <= UINT16_MAX){
			if(valOut)
				*valOut = (uint16_t) val;
			return true;
		}
	}
	return false;
}

bool  PumpHouseDB::getFloatProperty(string key, float * valOut){
	
	string str;
	if(getProperty(string(key), &str)){
		char* p;
		float val = strtof(str.c_str(), &p);
		if(*p == 0){
			if(valOut)
				*valOut = (float) val;
			return true;
		}
	}
	return false;
}
 
bool PumpHouseDB::restorePropertiesFromFile(string filePath){

	std::ifstream	ifs;
	bool 				statusOk = false;

	if(filePath.empty())
		filePath = _propertyFilePath;

	if(filePath.empty())
		filePath = defaultPropertyFilePath();
	
	try{
		string line;
		std::lock_guard<std::mutex> lock(_mutex);
	
		// open the file
		ifs.open(filePath, ios::in);
		if(!ifs.is_open()) return false;
	
		while ( std::getline(ifs, line) ) {
	
			// split the line looking for a token: and rest and ignore comments
			line = Utils::trimStart(line);
			if(line.size() == 0) continue;
			if(line[0] == '#')  continue;
			size_t pos = line.find(",");
			if(pos ==  string::npos) continue;
			
			string  key = line.substr(0, pos);
			string  value = line.substr(pos+1);
			value = Utils::trim(string(value));
			
			_properties[key] = value;
		}
		
		statusOk = true;
		ifs.close();
		
		// if we were sucessful, then save the filPath
		_propertyFilePath	= filePath;
	}
	catch(std::ifstream::failure &err) {
		
		LOG_INFO("restorePropertiesFromFile:FAIL: %s\n", err.what());
		statusOk = false;
	}
	
	return statusOk;
}

bool PumpHouseDB::savePropertiesToFile(string filePath){
 
	std::lock_guard<std::mutex> lock(_mutex);
	bool statusOk = false;
	
	std::ofstream ofs;
	
	if(filePath.empty())
		filePath = _propertyFilePath;

	if(filePath.empty())
		filePath = defaultPropertyFilePath();

	try{
		ofs.open(filePath, std::ios_base::trunc);
		
		if(ofs.fail())
			return false;
			
		for (auto& [key, value] : _properties) {
			ofs << key << ","  << value << "\n";
		}

		ofs.flush();
		ofs.close();
			
		statusOk = true;
	}
	catch(std::ofstream::failure &writeErr) {
			statusOk = false;
	}

		
	return statusOk;
}

string PumpHouseDB::defaultPropertyFilePath(){
	return "pumphouse.props.csv";
}




// MARK: -  API Secrets
bool PumpHouseDB::apiSecretCreate(string APIkey, string APISecret){
	
	return apiSecretSetSecret(APIkey,APISecret);
	 
}

bool PumpHouseDB::apiSecretSetSecret(string APIkey, string APISecret){
	
	if(!APIkey.empty() && !APISecret.empty()){
		setProperty(string(PROP_API_KEY), APIkey);
		setProperty(string(PROP_API_SECRET), APISecret);
		return true;
	}
	 
	return false;
}

bool PumpHouseDB::apiSecretDelete(string APIkey){
	
	if(!APIkey.empty()){
		removeProperty(string(PROP_API_KEY));
		removeProperty(string(PROP_API_SECRET));
		return true;
	}
	 
	return false;
}

bool PumpHouseDB::apiSecretGetSecret(string APIkey, string &APISecret){
	
	string key, secret;
	
	getProperty(string(PROP_API_KEY), &key);
	getProperty(string(PROP_API_SECRET), &secret);
 
	if(!key.empty() && !secret.empty()){
		
// we oinly have one in this DB
		if(key != APIkey)
			return false;

		APISecret = secret;
		return true;
	}
	
	return false;
}

bool PumpHouseDB::apiSecretMustAuthenticate(){
	return getProperty(string(PROP_API_KEY), NULL) &&  getProperty(string(PROP_API_SECRET),NULL);
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
	return 2021;
}

void  PumpHouseDB::setRESTPort(int port){
}

int PumpHouseDB::getRESTPort(){
	return 8081;
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

json PumpHouseDB::currentValuesJSON(eTag_t  eTag){
	json j;

	for (auto& [key, value] : _values) {
		
		if(eTag != 0){
			auto k = key;
			vector<string> v =  keysChangedSinceEtag(eTag);
			
 			bool found = std::any_of(v.begin(), v.end(),
											 [k](std::string const& s) {return s==k;});
			if(!found) continue;
		}
	
		auto lastpair = _values[key];
	
		time_t t = lastpair.first;
	 
		json entry;
		entry[string(JSON_ARG_VALUE)] 		=   jsonForValue(key, lastpair.second);
		entry[string(JSON_ARG_DISPLAYSTR)] =   displayStringForValue(key, lastpair.second);
		entry[string(JSON_ARG_TIME)] 		=   t;
 		j[key] = entry;
	}
	
	if(j.empty()){
		j = json::object();
	}
	
	return j;

 }


json PumpHouseDB::currentJSONForKey(string key){
	json j;

	if(_values.count(key)){
		auto lastpair = _values[key];

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

//	string suffix =  unitSuffixForKey(key);

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
			j = value;
//			double val = normalizedDoubleForValue(key, value);
//			j = to_string(val);
		}
			break;

		case SECONDS:
		case MINUTES:
		{
			int val  = intForValue(key, value);
			j = to_string(val);
		}
			break;
			
 		case VE_PRODUCT:
		{
			int val = intForValue(key, value);
			j = to_string(val);
		}
			break;

		default:
			j = value;
		break;
	}
	return j;
}

