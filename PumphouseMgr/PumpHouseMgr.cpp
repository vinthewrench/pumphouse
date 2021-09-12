//
//  PumpHouseMgr.cpp
//  wellpump
//
//  Created by Vincent Moscaritolo on 9/2/21.
//

#include "PumpHouseMgr.hpp"
#include "LogMgr.hpp"
#include "Utils.hpp"


const char* 	PumpHouseMgr::PumpHouseMgr_Version = "1.0.0 dev 1";

PumpHouseMgr::PumpHouseMgr(){
	// start the thread running
	_running = true;
	_state = PumpHouseDevice::DEVICE_STATE_UNKNOWN;
	_startTime = time(NULL);
	_thread = std::thread(&PumpHouseMgr::run, this);

}

PumpHouseMgr::~PumpHouseMgr(){
	_running = false;
	if (_thread.joinable())
		_thread.join();

}


//void PumpHouseMgr::start( std::function<void(bool didSucceed, string error_text)> cb){
//
//	LOGT_INFO("PumpHouseMgr %s START", PumpHouseMgr_Version);
//
//	if(!_db.initValueInfoFromFile("")){
//		if(cb)
//			(cb)(false,string("Failed to load db schema."));
//		return;;
//	}
//
//	startInverter();
//	startShunt();
// }

bool PumpHouseMgr::loadSetupFile(string filePath){
	
	bool success = false;
	_db.clear();
	success =  _db.initValueInfoFromFile(filePath);

	if(success) {
		_state = PumpHouseDevice::DEVICE_STATE_DISCONNECTED;
	}
 
	return success;
}


void PumpHouseMgr::stop(){
	
	LOGT_INFO("PumpHouseMgr STOP");
	stopInverter();
	stopShunt();
	
	_tempSensor1.stop();

}


void PumpHouseMgr::run(){
	
	constexpr long TIMEOUT_SEC = 3; //Timeout parameter for select() - in seconds
 
	try{
		
		while(_running){
			
			if(_tempSensor1.isConnected()){
				// handle input
				_tempSensor1.rcvResponse([=]( map<string,string> results){
					_db.insertValues(results);
				});
			}

			if(_smartshunt.isConnected() || _inverter.isConnected() ) {
			
				_state = PumpHouseDevice::DEVICE_STATE_CONNECTED;
				
				int max_sd = 0;
				struct timeval timeout = {TIMEOUT_SEC, 0};

				fd_set set;
				FD_ZERO (&set);
				
				if(_smartshunt.isConnected()){
					int fd = _smartshunt.getFD();
					FD_SET (fd,&set);
					if(fd > max_sd)
						max_sd = fd;
				}
				
				if(_inverter.isConnected()){

					int fd = _inverter.getFD();
					FD_SET (fd,&set);
					if(fd > max_sd)
						max_sd = fd;
				}
		 
				/* select returns 0 if timeout, 1 if input available, -1 if error. */
				int activity = select (max_sd + 1, &set, NULL, NULL,  &timeout);
				
				if ((activity < 0) && (errno!=EINTR)) {
						printf("select error");
				}
				
				if(_smartshunt.isConnected()){
					int fd = _smartshunt.getFD();
					if(FD_ISSET (fd,&set)){
	
						// handle input
						_smartshunt.rcvResponse([=]( map<string,string> results){
							_db.insertValues(results);
						});
					}
				}

				if(_inverter.isConnected()){
					int fd = _inverter.getFD();
					if(FD_ISSET (fd,&set)){
						// handle input
						_inverter.rcvResponse([=]( map<string,string> results){
							_db.insertValues(results);
						});
				}
				}
			}
			else { // no devices
				sleep(1);
			}
			
			_inverter.idle();
			_smartshunt.idle();
			_tempSensor1.idle();
		};
	}
	catch ( const SerialStreamException& e)  {
		printf("\tError %d %s\n\n", e.getErrorNumber(), e.what());
		
		if(e.getErrorNumber()	 == ENXIO){
	
		}
	}
}

// MARK: -   utilities

long PumpHouseMgr::upTime(){
	time_t now = time(NULL);

	return now - _startTime;
}



string PumpHouseMgr::deviceStateString(PumpHouseDevice::device_state_t st) {
	switch( st){
		case PumpHouseDevice::DEVICE_STATE_UNKNOWN:
			return "Unknown";
		case PumpHouseDevice::DEVICE_STATE_DISCONNECTED:
			return "Disconnected";
		case PumpHouseDevice::DEVICE_STATE_CONNECTED:
			return "Connected";
		case PumpHouseDevice::DEVICE_STATE_ERROR:
			return "Error";
	}
};


// MARK: -   Inverter

void PumpHouseMgr::startInverter( std::function<void(bool didSucceed, string error_text)> cb){
	int  errnum = 0;
	bool didSucceed = false;
	
	didSucceed =  _inverter.begin("/dev/ttyUSB1", &errnum);
	if(didSucceed)
		LOGT_INFO("Start Inverter  - OK");
	else
		LOGT_INFO("Start Inverter  - FAIL %s", string(strerror(errnum)).c_str());
 
	if(cb)
		(cb)(didSucceed, didSucceed?"": string(strerror(errnum) ));
}


void PumpHouseMgr::stopInverter(){
	_inverter.stop();
}

PumpHouseDevice::device_state_t PumpHouseMgr::inverterState(){
	return _inverter.getDeviceState();

}

// MARK: -   Battery Shunt

void PumpHouseMgr::startShunt( std::function<void(bool didSucceed, string error_text)> cb){
	
	int  errnum = 0;
	bool didSucceed = false;
	didSucceed =  _smartshunt.begin("/dev/ttyUSB0", &errnum);
	
	if(didSucceed)
		LOGT_INFO("Start Shunt  - OK");
	else
		LOGT_INFO("Start Shunt  - FAIL %s", string(strerror(errnum)).c_str());

	if(cb)
		(cb)(didSucceed, didSucceed?"": string(strerror(errnum) ));
}

										  
 void PumpHouseMgr::stopShunt(){
	 _smartshunt.stop();
 }
 
PumpHouseDevice::device_state_t PumpHouseMgr::shuntState(){
	return _smartshunt.getDeviceState();
}

// MARK: -   I2C Temp Sensors


void PumpHouseMgr::startTempSensor( std::function<void(bool didSucceed, std::string error_text)> cb){
	
	int  errnum = 0;
	bool didSucceed = false;
	
	
	uint8_t deviceAddress = 0x48;
	
	constexpr string_view TEMPSENSOR_KEY = "TEMP_";
	string resultKey =  string(TEMPSENSOR_KEY) + to_hex(deviceAddress,true);
 
	didSucceed =  _tempSensor1.begin(0x48,resultKey, &errnum);
	if(didSucceed){
		_db.addSchema(resultKey,
						  PumpHouseDB::DEGREES_C,
						  "Temp Sensor 1",
						  PumpHouseDB::TR_DONT_TRACK);
		
		LOGT_INFO("Start TempSensor  - OK");
	}
	else
		LOGT_INFO("Start TempSensor  - FAIL %s", string(strerror(errnum)).c_str());
 
	if(cb)
		(cb)(didSucceed, didSucceed?"": string(strerror(errnum) ));

}

void PumpHouseMgr::stopTempSensor(){
	_tempSensor1.stop();
}

PumpHouseDevice::device_state_t PumpHouseMgr::tempSensorState(){
	return _tempSensor1.getDeviceState();
}

