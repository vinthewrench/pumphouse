//
//  PumpHouseMgr.hpp
//  wellpump
//
//  Created by Vincent Moscaritolo on 9/2/21.
//

#ifndef PumpHouseMgr_hpp
#define PumpHouseMgr_hpp

#include <stdio.h>
#include <strings.h>
#include <unistd.h>
#include <thread>			//Needed for std::thread
#include <functional>

#include "PumpHouseException.hpp"
#include "SmartShunt.hpp"
#include "SigineerInverter.hpp"
#include "PumpHouseDB.hpp"


using namespace std;
 
class PumpHouseMgr {

public:
	static const char* 	PumpHouseMgr_Version;

	PumpHouseMgr();
	~PumpHouseMgr();
	
	void start( std::function<void(bool didSucceed, std::string error_text)> callback = NULL);
	
	void startInverter( std::function<void(bool didSucceed, std::string error_text)> callback = NULL);
	void startShunt( std::function<void(bool didSucceed, std::string error_text)> callback = NULL);

	void stop();
	void stopInverter();
	void stopShunt();

 private:
	 
	 bool 						_running;				//Flag for starting and terminating the main loop
	 std::thread 			_thread;				//Internal thread
	 
	 void run();

	SmartShunt			_smartshunt;
	SigineerInverter		_inverter;
	
	PumpHouseDB			_db;
	
};
#endif /* PumpHouseMgr_hpp */
