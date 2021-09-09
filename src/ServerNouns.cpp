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


static void Status_NounHandler(ServerCmdQueue* cmdQueue,
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
	if(noun != NOUN_STATUS){
		(completion) (reply, STATUS_NOT_FOUND);
		return;
	}
	
	reply[string(JSON_ARG_HAS_SHUNT)] = pumphouse.hasShunt();
	reply[string(JSON_ARG_HAS_INVERTER)] = pumphouse.hasInverter();

	auto state = pumphouse.currentState();
	string stateStr = pumphouse.currentStateString();
	
	reply[string(JSON_ARG_STATE)] 		=   state;
	reply[string(JSON_ARG_STATESTR)] 	=   stateStr;
	
	double temp;
	if(getCPUTemp(temp)){
		reply[string(JSON_ARG_CPU_TEMP)] =   temp;
	}

	makeStatusJSON(reply,STATUS_OK);
	(completion) (reply, STATUS_OK);
};


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
 
	reply[string(JSON_ARG_VALUES)] = db->currentValuesJSON();
 
	makeStatusJSON(reply,STATUS_OK);
	(completion) (reply, STATUS_OK);
};

 

// MARK: -  register server nouns

void registerServerNouns() {
	// create the server command processor
	auto cmdQueue = ServerCmdQueue::shared();

	cmdQueue->registerNoun(NOUN_VERSION, 	Version_NounHandler);
	cmdQueue->registerNoun(NOUN_DATE, 		Date_NounHandler);
	cmdQueue->registerNoun(NOUN_STATUS, 	Status_NounHandler);
	cmdQueue->registerNoun(NOUN_SCHEMA, 	Schema_NounHandler);
	cmdQueue->registerNoun(NOUN_VALUES, 	Schema_ValuesHandler);

}



