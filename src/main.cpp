//
//  main.cpp
//  wellpump
//
//  Created by Vincent Moscaritolo on 9/2/21.
//

#include <iostream>
#include "PumpHouseMgr.hpp"
#include "LogMgr.hpp"

[[clang::no_destroy]]  PumpHouseMgr	pumphouse;

int main(int argc, const char * argv[]) {
	
	LogMgr::shared()->_logFlags = LogMgr::LogLevelVerbose;
	
	try{
		
		pumphouse.start([=](bool didSucceed, string error){
		
			if(!didSucceed){
				printf("pumphouse.start fail- %s", error.c_str());
			}
		});
 
		// run the main loop.
		while(true) {
			sleep(60);
		}
		
	}	catch ( const PumpHouseException& e)  {
		printf("\tError %d %s\n\n", e.getErrorNumber(), e.what());
		
	}
	
	return 0;
}
