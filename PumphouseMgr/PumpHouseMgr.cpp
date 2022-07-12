//
//  PumpHouseMgr.cpp
//  wellpump
//
//  Created by Vincent Moscaritolo on 9/2/21.
//

#include "PumpHouseMgr.hpp"
#include "LogMgr.hpp"
#include "Utils.hpp"



const char* 	PumpHouseMgr::PumpHouseMgr_Version = "1.0.3 dev 1";

//void signal_callback_handler(int signum) {
//	cout << "Caught signal " << signum << endl;
//	// Terminate program
//	exit(signum);
//}

PumpHouseMgr::PumpHouseMgr():
								_tankSensor(&_db),
								_inverter(&_db),
								_pumpSensor(&_db){
//	signal(SIGINT, signal_callback_handler);
//	signal(SIGHUP, signal_callback_handler);
//	signal(SIGQUIT, signal_callback_handler);
//	signal(SIGTERM, signal_callback_handler);
//	signal(SIGKILL, signal_callback_handler);
//
	
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
		_db.logEvent(PumpHouseDB::EV_START );
		
	}
 
	return success;
}

void PumpHouseMgr::start(){
	initDataBase();
	
	startInverter();
 	startShunt();
	startTempSensor();
	startTankSensor();
	startPumpSensor();
}

void PumpHouseMgr::stop(){
	stopInverter();
	stopShunt();
	stopTankSensor();
	stopTempSensor();
	stopPumpSensor();
}


void PumpHouseMgr::setActiveConnections(bool isActive){
	_hasActiveConnections = isActive;
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

			if(_pumpSensor.isConnected()){
				// handle input
				_pumpSensor.rcvResponse([=]( map<string,string> results){
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
				
				// ping both devices for input
				if(_smartshunt.isConnected()){
						_smartshunt.rcvResponse([=]( map<string,string> results){
						_db.insertValues(results);
					});
				}
				
				if(_inverter.isConnected()){
					
					_inverter.rcvResponse([=]( map<string,string> results){
						_db.insertValues(results);
					});
				}
				
				if ((activity < 0) && (errno!=EINTR)) {
					LOGT_ERROR("SERIAL SELECT ERROR - FAIL %s", string(strerror(errno)).c_str());
				}
			}
			else { // no devices wait for timeout value
				sleep(TIMEOUT_SEC);
			}
			
			_inverter.setQueryDelay(_hasActiveConnections?60:5	);
 
			_inverter.idle();
			_smartshunt.idle();
			_tempSensor1.idle();
			_tempSensor2.idle();
			_tankSensor.idle();
			_pumpSensor.idle();
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
		case PumpHouseDevice::DEVICE_STATE_TIMEOUT:
			return "Timeout";

	}
};


// MARK: -   Inverter

void PumpHouseMgr::startInverter( std::function<void(bool didSucceed, string error_text)> cb){
	int  errnum = 0;
	bool didSucceed = false;
	string path;
	
	_db.setPropertyIfNone(string(PumpHouseDB::PROP_INVERTER_PORT), "/dev/ttyUSB1");
	_db.getProperty(string(PumpHouseDB::PROP_INVERTER_PORT), &path);
	_db.setPropertyIfNone(string(PumpHouseDB::PROP_INVERTER_BATV_FLOAT) ,to_string(26.0) );
	_db.setPropertyIfNone(string(PumpHouseDB::PROP_INVERTER_BATV_FAST),to_string(29.0) );
 
	if(path.empty()){
		if(cb) (cb)(false, string(PumpHouseDB::PROP_INVERTER_PORT) + " is empty" );
		return;
	}

	didSucceed =  _inverter.begin(path, &errnum);
	if(didSucceed) {
		LOGT_DEBUG("Start Inverter  - OK");
	}
	else{
		LOGT_ERROR("Start Inverter  - FAIL %s", string(strerror(errnum)).c_str());
	}
	
	// log the restart
	if(_inverter.isConnected()){
		_inverter.rcvResponse([=]( map<string,string> results){
			_db.insertValues(results);
		});
	}

	if(cb)
		(cb)(didSucceed, didSucceed?"": string(strerror(errnum) ));
}


void PumpHouseMgr::stopInverter(){
	_inverter.stop();
}

PumpHouseDevice::device_state_t PumpHouseMgr::inverterState(){
	return _inverter.getDeviceState();

}

time_t PumpHouseMgr::lastInverterReponseTime(){
	return _inverter.lastReponseTime();
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

	// log the restart
	if(_smartshunt.isConnected()){
		_smartshunt.rcvResponse([=]( map<string,string> results){
			_db.insertValues(results);
		});
	}

	if(cb)
		(cb)(didSucceed, didSucceed?"": string(strerror(errnum) ));
}

										  
 void PumpHouseMgr::stopShunt(){
	 _smartshunt.stop();
 }
 
PumpHouseDevice::device_state_t PumpHouseMgr::shuntState(){
	return _smartshunt.getDeviceState();
}

time_t PumpHouseMgr::lastShuntReponseTime(){
	return _smartshunt.lastReponseTime();
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
						  "Inverter Temperature",
						  PumpHouseDB::TR_TRACK);
		
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
						  "Battery Temperature",
						  PumpHouseDB::TR_TRACK);
		
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
	
	
	uint8_t deviceAddress = 0x69;
	
	const string TANK_DEPTH_KEY = "TANK";
	const string TANK_DEPTH_RAW_KEY = "TANK_RAW";

	string tank_trig_prefix = string(PumpHouseDB::PROP_TRIGGER_PREFIX)  + TANK_DEPTH_RAW_KEY;
	_db.setPropertyIfNone(tank_trig_prefix,to_string(20) );
	_db.setPropertyIfNone(string(PumpHouseDB::PROP_TANK_FULL) ,to_string(15400) );
	_db.setPropertyIfNone(string(PumpHouseDB::PROP_TANK_EMPTY),to_string(7000) );

	_db.addSchema(TANK_DEPTH_KEY,
					  PumpHouseDB::PERCENT,
					  "Tank Level",
					  PumpHouseDB::TR_TRACK);

	_db.addSchema(TANK_DEPTH_RAW_KEY,
					  PumpHouseDB::INT,
					  "Tank Level Raw",
					  PumpHouseDB::TR_TRACK);

	didSucceed =  _tankSensor.begin(deviceAddress, TANK_DEPTH_KEY, TANK_DEPTH_RAW_KEY, &errnum);
	if(didSucceed){
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

// MARK: -   Pump Sensor

void PumpHouseMgr::startPumpSensor( std::function<void(bool didSucceed, string error_text)> cb){
	
	int  errnum = 0;
	bool didSucceed = false;
	
	const string PUMP_SENSOR_KEY = "PUMP_SENSOR";

	didSucceed =  _pumpSensor.begin(0x27,PUMP_SENSOR_KEY, &errnum);
 
	if(didSucceed)
		LOGT_DEBUG("Start PumpSensor  - OK");
	else
		LOGT_ERROR("Start PumpSensor  - FAIL %s", string(strerror(errnum)).c_str());

	// log the restart
	if(_pumpSensor.isConnected()){
		_pumpSensor.rcvResponse([=]( map<string,string> results){
			_db.insertValues(results);
		});
	}

	if(cb)
		(cb)(didSucceed, didSucceed?"": string(strerror(errnum) ));
}

										  
 void PumpHouseMgr::stopPumpSensor(){
	 _pumpSensor.stop();
 }
 
PumpHouseDevice::device_state_t PumpHouseMgr::pumpSensorState(){
	return _pumpSensor.getDeviceState();
}
 
// MARK: - utilities

extern "C" {


void dumpHex(uint8_t* buffer, int length, int offset)
{
	char hexDigit[] = "0123456789ABCDEF";
	int			i;
	int						lineStart;
	int						lineLength;
	short					c;
	const unsigned char	  *bufferPtr = buffer;
	
	char                    lineBuf[1024];
	char                    *p;
	
#define kLineSize	8
	for (lineStart = 0, p = lineBuf; lineStart < length; lineStart += lineLength,  p = lineBuf )
	{
		lineLength = kLineSize;
		if (lineStart + lineLength > length)
			lineLength = length - lineStart;
		
		p += sprintf(p, "%6d: ", lineStart+offset);
		for (i = 0; i < lineLength; i++){
			*p++ = hexDigit[ bufferPtr[lineStart+i] >>4];
			*p++ = hexDigit[ bufferPtr[lineStart+i] &0xF];
			if((lineStart+i) &0x01)  *p++ = ' ';  ;
		}
		for (; i < kLineSize; i++)
			p += sprintf(p, "   ");
		
		p += sprintf(p,"  ");
		for (i = 0; i < lineLength; i++) {
			c = bufferPtr[lineStart + i] & 0xFF;
			if (c > ' ' && c < '~')
				*p++ = c ;
			else {
				*p++ = '.';
			}
		}
		*p++ = 0;
		
		
		printf("%s\n",lineBuf);
	}
#undef kLineSize
}
}
  
