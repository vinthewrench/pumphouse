//
//  main.cpp
//  wellpump
//
//  Created by Vincent Moscaritolo on 9/2/21.
//

#include <iostream>
#include "PumpHouseMgr.hpp"
#include "PumpHouseAPISecretMgr.hpp"

#include "CommonIncludes.h"
#include "LogMgr.hpp"

#include "TCPServer.hpp"
#include "Telnet/TelnetServerConnection.hpp"
#include "REST/RESTServerConnection.hpp"
#include "ServerCmdQueue.hpp"
#include "Telnet/CmdLineRegistry.hpp"

#include "ServerCommands.hpp"

[[clang::no_destroy]]  PumpHouseMgr	pumphouse;

int main(int argc, const char * argv[]) {
	
// 	LogMgr::shared()->_logFlags = LogMgr::LogLevelDebug;
 
	pumphouse.start();

	//set up the api secrets
	PumpHouseAPISecretMgr apiSecrets(pumphouse.getDB());
	
	int telnetPort = pumphouse.getDB()->getTelnetPort();
	int restPort = pumphouse.getDB()->getRESTPort();
	bool remoteTelnet = pumphouse.getDB()->getAllowRemoteTelnet();

	// create the server command processor
	auto cmdQueue = new ServerCmdQueue(&apiSecrets);
	registerServerNouns();
	registerCommandsLineFunctions();
	
	
	TCPServer telnet_server(cmdQueue);
	telnet_server.begin(telnetPort, true, [=](){
		return new TelnetServerConnection();
	});

	TCPServer rest_server(cmdQueue);
	rest_server.begin(restPort, remoteTelnet, [=](){
		return new RESTServerConnection();
	});

	
	// run the main loop.
	while(true) {

		pumphouse.setActiveConnections( rest_server.hasActiveConnections()
												|| telnet_server.hasActiveConnections());
 		sleep(2);
	}

	return 0;
}
