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

PumpHouseMgr::PumpHouseMgr(): _tankSensor(&_db){
	// start the thread running
	_running = true;
	_state = PumpHouseDevice::DEVICE_STATE_UNKNOWN;
	_startTime = time(NULL);
	_thread = std::thread(&PumpHouseMgr::run, this);
	_cpuInfo.begin();
}

PumpHouseMgr::~PumpHouseMgr(){
	_running = false;
	_cpuInfo.stop();
	
	if (_thread.joinable())
		_thread.join();
}


bool PumpHouseMgr::initDataBase(string schemaFilePath,
										  string logDBFilePath ){

	bool success = false;
	_db.clear();
 
	_db.restorePropertiesFromFile();

	// setup logfile path
	string str;
	_db.getProperty(string(PumpHouseDB::PROP_LOG_FILE), &str);
	LogMgr::shared()->setLogFilePath(str);

	_db.getProperty(string(PumpHouseDB::PROP_LOG_FLAGS), &str);
	if(!str.empty()){
		char* p;
		long val = strtol(str.c_str(), &p, 0);
		if(*p == 0){
			LogMgr::shared()->_logFlags = (uint8_t)val;
		}
	}
		
	success =  _db.initSchemaFromFile(schemaFilePath)
					&& _db.initLogDatabase(logDBFilePath);


	if(success) {
		_state = PumpHouseDevice::DEVICE_STATE_DISCONNECTED;
	}
 
	return success;
}

void PumpHouseMgr::start(){
	initDataBase();
	startInverter();
 	startShunt();
	startTempSensor();
	startTankSensor();
}

void PumpHouseMgr::stop(){
	stopInverter();
	stopShunt();
	stopTankSensor();
	stopTempSensor();
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

			if(_tempSensor2.isConnected()){
				// handle input
				_tempSensor2.rcvResponse([=]( map<string,string> results){
					_db.insertValues(results);
				});
			}

			if(_tankSensor.isConnected()){
				// handle input
				_tankSensor.rcvResponse([=]( map<string,string> results){
					_db.insertValues(results);
				});
			}
			
			if(_cpuInfo.isConnected()){
				// handle input
				_cpuInfo.rcvResponse([=]( map<string,string> results){
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
//						printf("select error");
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
			_tempSensor2.idle();
			_tankSensor.idle();
			_cpuInfo.idle();
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
	string path;
	
	_db.setPropertyIfNone(string(PumpHouseDB::PROP_INVERTER_PORT), "/dev/ttyUSB1");
	_db.getProperty(string(PumpHouseDB::PROP_INVERTER_PORT), &path);

	if(path.empty()){
		if(cb) (cb)(false, string(PumpHouseDB::PROP_INVERTER_PORT) + " is empty" );
		return;
	}

	didSucceed =  _inverter.begin(path, &errnum);
	if(didSucceed)
		LOGT_DEBUG("Start Inverter  - OK");
	else
		LOGT_ERROR("Start Inverter  - FAIL %s", string(strerror(errnum)).c_str());
 
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
	
	string path;
	
	_db.setPropertyIfNone(string(PumpHouseDB::PROP_SMARTSHUNT_PORT), "/dev/ttyUSB0");
	_db.getProperty(string(PumpHouseDB::PROP_SMARTSHUNT_PORT), &path);

	if(path.empty()){
		if(cb) (cb)(false, string(PumpHouseDB::PROP_SMARTSHUNT_PORT) + " is empty" );
		return;
	}
	
	didSucceed =  _smartshunt.begin(path, &errnum);
 
	if(didSucceed)
		LOGT_DEBUG("Start Shunt  - OK");
	else
		LOGT_ERROR("Start Shunt  - FAIL %s", string(strerror(errnum)).c_str());

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
	
	constexpr string_view TEMPSENSOR_KEY = "TEMP_";
	
	uint8_t deviceAddress = 0x48;
	string resultKey =  string(TEMPSENSOR_KEY) + to_hex(deviceAddress,true);
 
	didSucceed =  _tempSensor1.begin(deviceAddress,resultKey, &errnum);
	if(didSucceed){
		_db.addSchema(resultKey,
						  PumpHouseDB::DEGREES_C,
						  "Temp Sensor 1",
						  PumpHouseDB::TR_DONT_TRACK);
		
		LOGT_DEBUG("Start TempSensor 1 - OK");
	}
	else
		LOGT_ERROR("Start TempSensor 1  - FAIL %s", string(strerror(errnum)).c_str());
 
	
	deviceAddress = 0x49;
	resultKey =  string(TEMPSENSOR_KEY) + to_hex(deviceAddress,true);
 
	
	didSucceed =  _tempSensor2.begin(deviceAddress,resultKey, &errnum);
	if(didSucceed){
		_db.addSchema(resultKey,
						  PumpHouseDB::DEGREES_C,
						  "Temp Sensor 2",
						  PumpHouseDB::TR_DONT_TRACK);
		
		LOGT_DEBUG("Start TempSensor 2 - OK");
	}
	else
		LOGT_ERROR("Start TempSensor 2  - FAIL %s", string(strerror(errnum)).c_str());

	
	if(cb)
		(cb)(didSucceed, didSucceed?"": string(strerror(errnum) ));

}

void PumpHouseMgr::stopTempSensor(){
	_tempSensor1.stop();
	_tempSensor2.stop();
}

PumpHouseDevice::device_state_t PumpHouseMgr::tempSensor1State(){
	return _tempSensor1.getDeviceState();
}

PumpHouseDevice::device_state_t PumpHouseMgr::tempSensor2State(){
	return _tempSensor2.getDeviceState();
}


// MARK: -   Tank Sensor

 

void PumpHouseMgr::startTankSensor( std::function<void(bool didSucceed, std::string error_text)> cb){
	
	int  errnum = 0;
	bool didSucceed = false;
	
	
	uint8_t deviceAddress = 0x68;
	
	const string TANK_DEPTH_KEY = "TANK";
	const string TANK_DEPTH_RAW_KEY = "TANK_RAW";

	string tank_trig_prefix = string(PumpHouseDB::PROP_TRIGGER_PREFIX)  + TANK_DEPTH_RAW_KEY;
	_db.setPropertyIfNone(tank_trig_prefix,to_string(20) );
	_db.setPropertyIfNone(string(PumpHouseDB::PROP_TANK_FULL) ,to_string(15400) );
	_db.setPropertyIfNone(string(PumpHouseDB::PROP_TANK_EMPTY),to_string(7000) );

	didSucceed =  _tankSensor.begin(deviceAddress, TANK_DEPTH_KEY, TANK_DEPTH_RAW_KEY, &errnum);
	if(didSucceed){
		
		_db.addSchema(TANK_DEPTH_KEY,
						  PumpHouseDB::PERCENT,
						  "Tank Level",
						  PumpHouseDB::TR_DONT_TRACK);

		_db.addSchema(TANK_DEPTH_RAW_KEY,
						  PumpHouseDB::INT,
						  "Tank Level Raw",
						  PumpHouseDB::TR_DONT_TRACK);

		LOGT_INFO("Start TankSensor  - OK");
	}
	else
		LOGT_INFO("Start TankSensor  - FAIL %s", string(strerror(errnum)).c_str());
 
	if(cb)
		(cb)(didSucceed, didSucceed?"": string(strerror(errnum) ));

}

void PumpHouseMgr::stopTankSensor(){
	_tankSensor.stop();
}

PumpHouseDevice::device_state_t PumpHouseMgr::tankSensorState(){
	return _tankSensor.getDeviceState();
}
