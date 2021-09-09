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
// MARK: - COMMAND LINE FUNCTIONS

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
		long 	uptime;
 
		ServerCmdArgValidator v1;

		v1.getLongIntFromJSON("uptime", reply, uptime);
	
		if(v1.getStringFromJSON(JSON_ARG_DATE, reply, str)){
			using namespace timestamp;
			
			time_t tt =  TimeStamp(str).getTime();
			
			oss << setw(10) << "TIME: " << setw(0) <<  TimeStamp(tt).ClockString(false);
			oss << "\n\r";
		}
		
		if(uptime){
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
 
	CmdLineHelp::shared()->setHelpFile("helpfile.txt");
}
 
