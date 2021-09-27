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

#include "PumphouseCommon.hpp"
#include "PumpHouseDB.hpp"
#include "SmartShunt.hpp"
#include "SigineerInverter.hpp"
#include "TempSensor.hpp"
#include "TankDepth.hpp"
#include "CPUInfo.hpp"

using namespace std;
 
class PumpHouseMgr {

public:
	static const char* 	PumpHouseMgr_Version;

 	PumpHouseMgr();
	~PumpHouseMgr();
 
	void start();
	void stop();
	
	void startInverter( std::function<void(bool didSucceed, std::string error_text)> callback = NULL);
	void stopInverter();
	PumpHouseDevice::device_state_t inverterState();

	void startShunt( std::function<void(bool didSucceed, std::string error_text)> callback = NULL);
	void stopShunt();
	PumpHouseDevice::device_state_t shuntState();
	
	void startTempSensor( std::function<void(bool didSucceed, std::string error_text)> callback = NULL);
	void stopTempSensor();
	PumpHouseDevice::device_state_t tempSensor1State();
	PumpHouseDevice::device_state_t tempSensor2State();

	void startTankSensor( std::function<void(bool didSucceed, std::string error_text)> callback = NULL);
	void stopTankSensor();
	PumpHouseDevice::device_state_t tankSensorState();
	
	string deviceStateString(PumpHouseDevice::device_state_t st);
 
	bool initDataBase(string schemaFilePath = "", string logDBFilePath = "");

	long upTime();  // how long since we started
	
	PumpHouseDevice::device_state_t pumpHouseState() {return _state;};

	PumpHouseDB*		getDB() {return &_db; };
 
	
 private:
	 
	 bool 						_running;				//Flag for starting and terminating the main loop
	 std::thread 			_thread;				//Internal thread
	 
	 void run();

	PumpHouseDevice::device_state_t	_state;
	time_t 									_startTime;		// to calculate uptime

	SmartShunt			_smartshunt;
	SigineerInverter		_inverter;
	TempSensor			_tempSensor1;
	TempSensor			_tempSensor2;

	TankDepth			_tankSensor;
	CPUInfo				_cpuInfo;
	PumpHouseDB			_db;
	
	
};
#endif /* PumpHouseMgr_hpp */
