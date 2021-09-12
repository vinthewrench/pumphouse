//
//  ServerCommandsLineFunctions.cpp
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
#include "Telnet/TelnetServerConnection.hpp"
#include "REST/RESTServerConnection.hpp"
#include "ServerCmdQueue.hpp"
#include "Telnet/CmdLineRegistry.hpp"

#include "ServerCommands.hpp"
#include "ServerCmdValidators.hpp"
#include "CommonIncludes.h"

#include "LogMgr.hpp"

#include "Utils.hpp"
 

/* Useful Constants */
#define SECS_PER_MIN  ((time_t)(60UL))
#define SECS_PER_HOUR ((time_t)(3600UL))
#define SECS_PER_DAY  ((time_t)(SECS_PER_HOUR * 24UL))
#define DAYS_PER_WEEK ((time_t)(7UL))
#define SECS_PER_WEEK ((time_t)(SECS_PER_DAY * DAYS_PER_WEEK))
#define SECS_PER_YEAR ((time_t)(SECS_PER_DAY * 365UL)) // TODO: ought to handle leap years
#define SECS_YR_2000  ((time_t)(946684800UL)) // the time at the start of y2k
 
static void breakDuration(unsigned long secondsIn, tm &tm){
	
	long  remainingSeconds = secondsIn;
		
	tm.tm_mday =  (int)(remainingSeconds/SECS_PER_DAY);
	remainingSeconds = secondsIn - (tm.tm_mday * SECS_PER_DAY);
	
	tm.tm_hour =  (int)(remainingSeconds/SECS_PER_HOUR);
	remainingSeconds = secondsIn -   ((tm.tm_mday * SECS_PER_DAY) + (tm.tm_hour * SECS_PER_HOUR));
	
	tm.tm_min = (int)remainingSeconds/SECS_PER_MIN;
	remainingSeconds = remainingSeconds - (tm.tm_min * SECS_PER_MIN);
	
	tm.tm_sec = (int) remainingSeconds;
}


typedef struct {
	string  					description;
	string					  	suffix;
} schemaEntry_t;

static void getSchema( CmdLineMgr* mgr,
									std::function<void(map<string, schemaEntry_t>)> callback = NULL) {
	using namespace rest;

	TCPClientInfo cInfo = mgr->getClientInfo();
	ServerCmdQueue::shared()->queueRESTCommand(REST_URL("GET /schema\n\n")
															 , cInfo,[=] (json reply, httpStatusCodes_t code) {
		bool success = didSucceed(reply);

		map<string, schemaEntry_t> schemaMap;
		schemaMap.clear();
		
		if(success){
			
			string key2 = string(JSON_ARG_SCHEMA);
			
			if( reply.contains(key2)
				&& reply.at(key2).is_object()){
				auto entries = reply.at(key2);
	 
				 for (auto& [key, value] : entries.items()) {
		 
					 schemaEntry_t schemaEntry;
				 
					 if( value.contains(string(JSON_ARG_NAME))
						 && value.at(string(JSON_ARG_NAME)).is_string()){
						 schemaEntry.description  = value.at(string(JSON_ARG_NAME));
					 };
					 
					 if( value.contains(string(JSON_ARG_SUFFIX))
						 && value.at(string(JSON_ARG_SUFFIX)).is_string()){
						 schemaEntry.suffix  = value.at(string(JSON_ARG_SUFFIX));
					 };
		
					 schemaMap[key] = schemaEntry;
				 }
			}
		}
	
		if(callback) (callback)(schemaMap);
		
	});
}

static bool LiSTCmdHandler( stringvector line,
										CmdLineMgr* mgr,
										boolCallback_t	cb){
	using namespace rest;
	TCPClientInfo cInfo = mgr->getClientInfo();
	 
	 getSchema(mgr, [=](map<string, schemaEntry_t> schema){
		 
		 REST_URL url("GET /values\n\n");
		 ServerCmdQueue::shared()->queueRESTCommand(url, cInfo,[=] (json reply, httpStatusCodes_t code) {
			 
			 bool success = didSucceed(reply);
			 auto sMap = schema;
	
		
			 if(success) {
				 std::ostringstream oss;
			
				 typedef struct {
					 string  		displayString;
	 			 } valueEntry_t;

				 map<string, valueEntry_t> valueMap;
				 valueMap.clear();
		 
				 string key1 = string(JSON_ARG_VALUES);
				 
				 if( reply.contains(key1)
					 && reply.at(key1).is_object()){
					 auto entries = reply.at(key1);
					 
					 for (auto& [key, entry] : entries.items()) {
			 
						 valueEntry_t valEntry;
						 
						 if( entry.contains(string(JSON_ARG_DISPLAYSTR))
							 && entry.at(string(JSON_ARG_DISPLAYSTR)).is_string()){
							 valEntry.displayString  = entry.at(string(JSON_ARG_DISPLAYSTR));
						 };
						 
						 valueMap[key] = valEntry;
					 };
				 
				 }
				
				 vector<string> keys;
				 for (auto& [key, _] : valueMap) {
					 keys.push_back(key);
				 }
				 
				 sort(keys.begin(), keys.end());
	
		
				 for (auto key  : keys) {
					 
					 auto value = valueMap[key];
					 schemaEntry_t s = sMap[key];
 
					 oss << setiosflags(ios::left) <<  std::setw(10) << key << " " << std::setw(16)
					 	<<  value.displayString
					 	<< std::setw(0)  << " " << s.description   << "\n\r";
 				 }
				 oss << "\r\n";
				 mgr->sendReply(oss.str());

				 
			 }
			 else {
				 string error = errorMessage(reply);
				 mgr->sendReply( error + "\n\r");
			 }
			 
			 (cb) (success);
			 
		 });
	 
	 });
		 
 	return true;
};


// MARK: - COMMAND LINE FUNCTIONS



static bool STATUSCmdHandler( stringvector line,
										CmdLineMgr* mgr,
										boolCallback_t	cb){
	using namespace rest;
	TCPClientInfo cInfo = mgr->getClientInfo();

	// simulate URL
	REST_URL url("GET /state\n\n");
	ServerCmdQueue::shared()->queueRESTCommand(url, cInfo,[=] (json reply, httpStatusCodes_t code) {

		bool success = didSucceed(reply);
		
		if(success) {
			std::ostringstream oss;
			
			if(reply.count(JSON_ARG_STATE) ) {
				auto state = reply[string(JSON_ARG_STATE)];
				oss  << setw(11) << "STATE: " <<  setw(0) << state << "\n\r";
			}
	
			if(reply.count(JSON_ARG_INVERTER) ) {
				auto state = reply[string(JSON_ARG_INVERTER)];
				oss  << setw(11) << "INVERTER: " <<  setw(0) << state << "\n\r";
			}
	
			if(reply.count(JSON_ARG_BATTERY) ) {
				auto state = reply[string(JSON_ARG_BATTERY)];
				oss  << setw(11) << "BATTERY: " <<  setw(0) << state << "\n\r";
			}
	
			oss << "\r\n";
			mgr->sendReply(oss.str());
			
		}
		else {
			string error = errorMessage(reply);
			mgr->sendReply( error + "\n\r");
		}
		
		(cb) (success);

	});
	
	return true;
};



static bool STARTSTOPCmdHandler( stringvector line,
										CmdLineMgr* mgr,
										boolCallback_t	cb){
	using namespace rest;
	TCPClientInfo cInfo = mgr->getClientInfo();

	using namespace rest;
	string errorStr;
	string command = line[0];
	
 	std::ostringstream oss;
	REST_URL url;

	if(line.size() < 2){
		errorStr =  "Command: \x1B[36;1;4m"  + command + "\x1B[0m expects a device.";
	}
	else{
		string subcommand = line[1];
		std::transform(subcommand.begin(), subcommand.end(), subcommand.begin(), ::tolower);

		if(subcommand == "battery" || subcommand == "inverter"){
			
			oss << "PUT /state/" <<  subcommand << " HTTP/1.1\n";
			
			oss << "Content-Type: application/json; charset=utf-8\n";
			oss << "Connection: close\n";
			
			json request;
			
			if(command == "start"){
				request[string(JSON_ARG_STATE)] =  JSON_VAL_START;
			}
			else if(command == "stop"){
				request[string(JSON_ARG_STATE)] =  JSON_VAL_STOP;
			}
			
			string jsonStr = request.dump(4);
			oss << "Content-Length: " << jsonStr.size() << "\n\n";
			oss << jsonStr << "\n";
			url.setURL(oss.str());
			
			ServerCmdQueue::shared()->queueRESTCommand(url, cInfo,[=] (json reply, httpStatusCodes_t code) {
				
				bool success = didSucceed(reply);
				
				if(success) {
					mgr->sendReply( "OK \n\r");
				}
				else {
					string error = errorMessage(reply);
					mgr->sendReply( error + "\n\r");
				}
				
				(cb) (success);
				
			});
			
			return true;
			
		}
		else {
			
			errorStr =  "\x1B[36;1;4m"  + command + "\x1B[0m does not recognize the  device: \x1B[33;1m" + subcommand  + "\x1B[0m";
		}

	}
 
	mgr->sendReply(errorStr + "\n\r");
	(cb) (false);
	return false;
};

static bool VersionCmdHandler( stringvector line,
										CmdLineMgr* mgr,
										boolCallback_t	cb){
	using namespace rest;
	TCPClientInfo cInfo = mgr->getClientInfo();
	//
	//	for (auto& t : cInfo.headers()){
	//		printf("%s = %s\n", t.first.c_str(), t.second.c_str());
	//	}
	
	// simulate URL
	REST_URL url("GET /version\n\n");
	ServerCmdQueue::shared()->queueRESTCommand(url, cInfo,[=] (json reply, httpStatusCodes_t code) {
		
		bool success = didSucceed(reply);
		
		if(success) {
			std::ostringstream oss;
			
			if(reply.count(JSON_ARG_VERSION) ) {
				string ver = reply[string(JSON_ARG_VERSION)];
				oss << ver << ", ";
			}
			
			if(reply.count(JSON_ARG_TIMESTAMP) ) {
				string timestamp = reply[string(JSON_ARG_TIMESTAMP)];
				oss <<  timestamp;
			}
			
			oss << "\r\n";
			mgr->sendReply(oss.str());
			
		}
		else {
			string error = errorMessage(reply);
			mgr->sendReply( error + "\n\r");
		}
		
		(cb) (success);
		
	});
	
	return true;
};



static bool WelcomeCmdHandler( stringvector line,
										CmdLineMgr* mgr,
										boolCallback_t	cb){
	
	std::ostringstream oss;
	
	// add friendly info here
	oss << "Welcome to the Pumphouse Manager: ";
	mgr->sendReply(oss.str());

	VersionCmdHandler( {"version"}, mgr, cb);
	return true;
}


static bool DATECmdHandler( stringvector line,
									CmdLineMgr* mgr,
									boolCallback_t	cb){
	using namespace rest;
	TCPClientInfo cInfo = mgr->getClientInfo();
	
	REST_URL url("GET /date\n\n");
	ServerCmdQueue::shared()->queueRESTCommand(url, cInfo,[=] (json reply, httpStatusCodes_t code) {
	
		std::ostringstream oss;

		string str;
		long 	uptime = 0;
 
		ServerCmdArgValidator v1;

		if(v1.getStringFromJSON(JSON_ARG_DATE, reply, str)){
			using namespace timestamp;
			
			time_t tt =  TimeStamp(str).getTime();
			
			oss << setw(10) << "TIME: " << setw(0) <<  TimeStamp(tt).ClockString(false);
			oss << "\n\r";
		}
		
		if(	v1.getLongIntFromJSON("uptime", reply, uptime)){
			char timeStr[80] = {0};
			tm tm;
			breakDuration(uptime, tm);
	 
			if(tm.tm_mday > 0){
				
				sprintf(timeStr, "%d %s, %01d:%02d:%02d" ,
								  tm.tm_mday, (tm.tm_mday>1?"Days":"Day"),
								  tm.tm_hour, tm.tm_min, tm.tm_sec);
			}
			else {
				sprintf(timeStr, "%01d:%02d:%02d" ,
								  tm.tm_hour, tm.tm_min, tm.tm_sec);
			}

			oss << setw(10) << "UPTIME: " << setw(0) << timeStr <<  "\n\r";;
		}
		
 
		oss << "\r\n";
		mgr->sendReply(oss.str());

		(cb) (code > 199 && code < 400);
	});
	
	//cmdQueue->queue
	
	return true;
};

// MARK: -  register commands

void registerCommandsLineFunctions() {
	
	// register command line commands
	auto cmlR = CmdLineRegistry::shared();
	
	cmlR->registerCommand(CmdLineRegistry::CMD_WELCOME ,	WelcomeCmdHandler);
	
	cmlR->registerCommand("version",	VersionCmdHandler);
	cmlR->registerCommand("date",		DATECmdHandler);
	cmlR->registerCommand("list",		LiSTCmdHandler);
 
	cmlR->registerCommand("status",		STATUSCmdHandler);
	cmlR->registerCommand("start",		STARTSTOPCmdHandler);
	cmlR->registerCommand("stop",		STARTSTOPCmdHandler);
 
	CmdLineHelp::shared()->setHelpFile("helpfile.txt");
}
 
