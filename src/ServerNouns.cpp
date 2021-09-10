//
//  ServerNouns.cpp
//  pumphouse
//
//  Created by Vincent Moscaritolo on 9/9/21.
//

#include <stdio.h>
#include <iostream>
#include <chrono>

#include "ServerCmdQueue.hpp"
#include "CmdLineHelp.hpp"
#include <regex>
#include <string>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <sstream>
#include <unistd.h>			//Needed for access()
#include <string>
#include <string.h>
#include <map>

#include "Utils.hpp"
#include "TimeStamp.hpp"

#include "TCPServer.hpp"
#include "REST/RESTServerConnection.hpp"
#include "ServerCmdQueue.hpp"

#include "ServerCommands.hpp"
#include "ServerCmdValidators.hpp"
#include "CommonIncludes.h"

#include "LogMgr.hpp"
#include "Utils.hpp"


#include "PumpHouseMgr.hpp"

static bool getCPUTemp(double &tempOut) {
	bool didSucceed = false;

#if defined(__PIE__)
	// return the CPU temp
		{
			try{
				std::ifstream   ifs;
				ifs.open("/sys/class/thermal/thermal_zone0/temp", ios::in);
				if( ifs.is_open()){
					string val;
					ifs >> val;
					ifs.close();
					double temp = std::stod(val);
					temp = temp /1000.0;
					tempOut = temp;
					didSucceed = true;
				}
				
			}
			catch(std::ifstream::failure &err) {
			}
		}
#else
//	tempOut = 38.459;
//	didSucceed = true;

#endif
	
	return didSucceed;
}

// MARK:  OTHER REST NOUN HANDLERS


static void Version_NounHandler(ServerCmdQueue* cmdQueue,
										  REST_URL url,
										  TCPClientInfo cInfo,
										  ServerCmdQueue::cmdCallback_t completion) {
	
	using namespace rest;
	json reply;
	
	// CHECK METHOD
	if(url.method() != HTTP_GET ) {
		(completion) (reply, STATUS_INVALID_METHOD);
		return;
	}
	
	auto path = url.path();
	string noun;
	
	if(path.size() > 0) {
		noun = path.at(0);
	}
	
	// CHECK sub paths
	if(noun != NOUN_VERSION){
		(completion) (reply, STATUS_NOT_FOUND);
		return;
	}
	
	reply[string(JSON_ARG_VERSION)] = PumpHouseMgr::PumpHouseMgr_Version;
	reply[string(JSON_ARG_TIMESTAMP)]	=  string(__DATE__) + " " + string(__TIME__);
	
	makeStatusJSON(reply,STATUS_OK);
	(completion) (reply, STATUS_OK);
};

static void Date_NounHandler(ServerCmdQueue* cmdQueue,
									  REST_URL url,
									  TCPClientInfo cInfo,
									  ServerCmdQueue::cmdCallback_t completion) {
	
	using namespace rest;
	using namespace timestamp;
	json reply;
	string plmFunc;

	// CHECK METHOD
	if(url.method() != HTTP_GET ) {
		(completion) (reply, STATUS_INVALID_METHOD);
		return;
	}
	
	auto path = url.path();
	string noun;
	
	if(path.size() > 0) {
		noun = path.at(0);
	}
	
	// CHECK sub paths
	if(noun != NOUN_DATE){
		(completion) (reply, STATUS_NOT_FOUND);
		return;
	}
	
	reply["date"] = TimeStamp().RFC1123String();
	reply["uptime"]	= pumphouse.upTime();
	
	makeStatusJSON(reply,STATUS_OK);
	(completion) (reply, STATUS_OK);
}


static void Schema_NounHandler(ServerCmdQueue* cmdQueue,
										  REST_URL url,
										  TCPClientInfo cInfo,
										  ServerCmdQueue::cmdCallback_t completion) {
	
	using namespace rest;
	json reply;
	
	auto db = pumphouse.getDB();
	
	// CHECK METHOD
	if(url.method() != HTTP_GET ) {
		(completion) (reply, STATUS_INVALID_METHOD);
		return;
	}
	
	auto path = url.path();
	string noun;
	
	if(path.size() > 0) {
		noun = path.at(0);
	}
	
	// CHECK sub paths
	if(noun != NOUN_SCHEMA){
		(completion) (reply, STATUS_NOT_FOUND);
		return;
	}
 
 	reply[string(JSON_ARG_SCHEMA)] = db->schemaJSON();;
 
	makeStatusJSON(reply,STATUS_OK);
	(completion) (reply, STATUS_OK);
};

static void Schema_ValuesHandler(ServerCmdQueue* cmdQueue,
										  REST_URL url,
										  TCPClientInfo cInfo,
										  ServerCmdQueue::cmdCallback_t completion) {
	
	using namespace rest;
	json reply;
	ServerCmdArgValidator v1;
	string str;
	
	auto db = pumphouse.getDB();
	
	// CHECK METHOD
	if(url.method() != HTTP_GET ) {
		(completion) (reply, STATUS_INVALID_METHOD);
		return;
	}
	
	auto path = url.path();
	string noun;
	
	if(path.size() > 0) {
		noun = path.at(0);
	}
	
	// CHECK sub paths
	if(noun != NOUN_VALUES){
		(completion) (reply, STATUS_NOT_FOUND);
		return;
	}
 
	eTag_t eTag = 0;
	
	if(v1.getStringFromMap("If-None-Match", url.headers(), str)){
		char* p;
		eTag = strtol(str.c_str(), &p, 0);
		if(*p != 0) eTag = 0;
	}
		
	reply[string(JSON_ARG_VALUES)] = db->currentValuesJSON(eTag);
	reply[string(JSON_ARG_ETAG)] = db->lastEtag();

	makeStatusJSON(reply,STATUS_OK);
	(completion) (reply, STATUS_OK);
};

 
// MARK:  EVENTS NOUN HANDLERS

static bool State_NounHandler_GET(ServerCmdQueue* cmdQueue,
											  REST_URL url,
											  TCPClientInfo cInfo,
											  ServerCmdQueue::cmdCallback_t completion) {
	using namespace rest;
	json reply;
	
	auto path = url.path();
	string noun;
	
	if(path.size() > 0) {
		noun = path.at(0);
	}
	
	// CHECK sub paths
	if(noun != NOUN_STATE){
		(completion) (reply, STATUS_NOT_FOUND);
		return false;
	}
	 
	reply[string(JSON_ARG_STATE)] 		= pumphouse.pumpHouseState();
	reply[string(JSON_ARG_INVERTER)] 	= pumphouse.inverterState() ;
	reply[string(JSON_ARG_BATTERY)] 	= pumphouse.shuntState() ;

	double temp;
	if(getCPUTemp(temp)){
		reply[string(JSON_ARG_CPU_TEMP)] =   temp;
	}

	makeStatusJSON(reply,STATUS_OK);
	(completion) (reply, STATUS_OK);
	return true;
}


static bool State_NounHandler_PUT(ServerCmdQueue* cmdQueue,
											  REST_URL url,
											  TCPClientInfo cInfo,
											  ServerCmdQueue::cmdCallback_t completion) {
	
	using namespace rest;
	auto path = url.path();
	auto queries = url.queries();
	auto headers = url.headers();
	
	ServerCmdArgValidator v1;
	//	auto db = pumphouse.getDB();
	
	string noun;
	json reply;
	
	if(path.size() > 0) {
		noun = path.at(0);
	}
	
	if(path.size() == 2) {
		
		string subpath =   path.at(1);
		string str;
		
		if( subpath == SUBPATH_INVERTER) {
			
			PumpHouseDevice::device_state_t devState =  pumphouse.inverterState() ;
			
			if(v1.getStringFromJSON(JSON_ARG_STATE, url.body(), str)){
				std::transform(str.begin(), str.end(), str.begin(), ::tolower);
				
				if(str == JSON_VAL_START){
					
					if(devState == PumpHouseDevice::DEVICE_STATE_CONNECTED){
						makeStatusJSON(reply,
											STATUS_BAD_REQUEST, "Inverter in wrong state",
											"Inverter is " + pumphouse.deviceStateString(devState)  );
						(completion) (reply, STATUS_BAD_REQUEST);
						return true;
					}
					
					pumphouse.startInverter([=](bool didSucceed, string error_text){
						json reply;
						
						if(didSucceed){
							makeStatusJSON(reply,STATUS_OK);
							(completion) (reply, STATUS_OK);
						}
						else {
							makeStatusJSON(reply, STATUS_BAD_REQUEST, "Start Failed", error_text );
							(completion) (reply, STATUS_BAD_REQUEST);
						}
					});
					return true;
					
				}else 	if(str == JSON_VAL_STOP){
					
					if(devState == PumpHouseDevice::DEVICE_STATE_DISCONNECTED){
						makeStatusJSON(reply,
											STATUS_BAD_REQUEST, "Inverter in wrong state",
											"Inverter is " + pumphouse.deviceStateString(devState) );
						(completion) (reply, STATUS_BAD_REQUEST);
						return true;
					}
					else {
						pumphouse.stopInverter();
						makeStatusJSON(reply,STATUS_OK);
						(completion) (reply, STATUS_OK);
						return true;
					}
				}
			}
		 
		}
		else if( subpath == SUBPATH_BATTERY) {
			
			PumpHouseDevice::device_state_t devState =  pumphouse.shuntState() ;
			
			if(v1.getStringFromJSON(JSON_ARG_STATE, url.body(), str)){
				std::transform(str.begin(), str.end(), str.begin(), ::tolower);
				
				if(str == JSON_VAL_START){
					
					if(devState == PumpHouseDevice::DEVICE_STATE_CONNECTED){
						makeStatusJSON(reply,
											STATUS_BAD_REQUEST, "Battery in wrong state",
											"Battery is " + pumphouse.deviceStateString(devState)  );
						(completion) (reply, STATUS_BAD_REQUEST);
						return true;
					}
					
					pumphouse.startShunt([=](bool didSucceed, string error_text){
						json reply;
						
						if(didSucceed){
							makeStatusJSON(reply,STATUS_OK);
							(completion) (reply, STATUS_OK);
						}
						else {
							makeStatusJSON(reply, STATUS_BAD_REQUEST, "Start Failed", error_text );
							(completion) (reply, STATUS_BAD_REQUEST);
						}
					});
					return true;
					
				}else 	if(str == JSON_VAL_STOP){
					
					if(devState == PumpHouseDevice::DEVICE_STATE_DISCONNECTED){
						makeStatusJSON(reply,
											STATUS_BAD_REQUEST, "Battery in wrong state",
											"Battery is " + pumphouse.deviceStateString(devState)  );
						(completion) (reply, STATUS_BAD_REQUEST);
						return true;
					}
					else {
						pumphouse.stopShunt();
						makeStatusJSON(reply,STATUS_OK);
						(completion) (reply, STATUS_OK);
						return true;
					}
				}
			}
			
		}
	}
	
	return false;
}


static void State_NounHandler(ServerCmdQueue* cmdQueue,
										  REST_URL url,
										  TCPClientInfo cInfo,
										  ServerCmdQueue::cmdCallback_t completion) {

	using namespace rest;
	json reply;
	
	auto path = url.path();
	auto queries = url.queries();
	auto headers = url.headers();
	string noun;
	
	bool isValidURL = false;
	
	if(path.size() > 0) {
		noun = path.at(0);
	}

	switch(url.method()){
		case HTTP_GET:
			isValidURL = State_NounHandler_GET(cmdQueue,url,cInfo, completion);
			break;
			
		case HTTP_PUT:
			isValidURL = State_NounHandler_PUT(cmdQueue,url,cInfo, completion);
			break;

//		case HTTP_PATCH:
//			isValidURL = State_NounHandler_PATCH(cmdQueue,url,cInfo, completion);
//			break;
//
//		case HTTP_POST:
//			isValidURL = State_NounHandler_POST(cmdQueue,url,cInfo, completion);
//			break;
//
//		case HTTP_DELETE:
//			isValidURL = Events_NounHandler_DELETE(cmdQueue,url,cInfo, completion);
//			break;
  
		default:
			(completion) (reply, STATUS_INVALID_METHOD);
			return;
	}
	
	if(!isValidURL) {
		(completion) (reply, STATUS_NOT_FOUND);
	}
};


// MARK: -  register server nouns

void registerServerNouns() {
	// create the server command processor
	auto cmdQueue = ServerCmdQueue::shared();

	cmdQueue->registerNoun(NOUN_VERSION, 	Version_NounHandler);
	cmdQueue->registerNoun(NOUN_DATE, 		Date_NounHandler);
	cmdQueue->registerNoun(NOUN_STATE, 	State_NounHandler);
	cmdQueue->registerNoun(NOUN_SCHEMA, 	Schema_NounHandler);
	cmdQueue->registerNoun(NOUN_VALUES, 	Schema_ValuesHandler);

}



