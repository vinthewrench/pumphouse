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

	typedef enum  {
		STATE_UNKNOWN = 0,
		STATE_INIT,
		STATE_SETUP,
		STATE_READY,
	
	}mgr_state_t;
	
	PumpHouseMgr();
	~PumpHouseMgr();
	
//	void start( std::function<void(bool didSucceed, std::string error_text)> callback = NULL);
	
	void startInverter( std::function<void(bool didSucceed, std::string error_text)> callback = NULL);
	void startShunt( std::function<void(bool didSucceed, std::string error_text)> callback = NULL);
	void stopInverter();
	void stopShunt();
	bool hasInverter(){return _inverter.isConnected();};
	bool hasShunt(){return _smartshunt.isConnected();};

	void stop();

 	bool loadSetupFile(string filePath = "");
	
	long upTime();  // how long since we started
	
	mgr_state_t currentState() {return _state;};
	string currentStateString();

	PumpHouseDB*			getDB() {return &_db; };
 
 private:
	 
	 bool 						_running;				//Flag for starting and terminating the main loop
	 std::thread 			_thread;				//Internal thread
	 
	 void run();

	mgr_state_t				_state;
	time_t 					_startTime;		// to calculate uptime

	SmartShunt			_smartshunt;
	SigineerInverter		_inverter;
	
	PumpHouseDB			_db;
	
};
#endif /* PumpHouseMgr_hpp */
