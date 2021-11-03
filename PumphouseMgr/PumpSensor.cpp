//
//  PumpSensor.cpp
//  pumphouse
//
//  Created by Vincent Moscaritolo on 10/29/21.
//

#include "PumpSensor.hpp"
#include "LogMgr.hpp"

#include <bitset>


PumpSensor::PumpSensor( PumpHouseDB *db){
	_db = db;
	_state = INS_UNKNOWN;
	_lastQueryTime = {0,0};
	_resultMap.clear();
	 
}

PumpSensor::~PumpSensor(){
	stop();
}
 
bool PumpSensor::begin(int deviceAddress, string resultKey, int *error){
	bool status = false;
	
	status = _sensor.begin(deviceAddress, error);
	
	if(status){
		
		_sensor.writeConfig(0xff);
 		_sensor.writeInvert( 0xFF);
	
		
		_state = INS_IDLE;
		_queryDelay =  5;	// seconds
		_lastQueryTime = {0,0};
		_resultMap.clear();
		_resultKey = resultKey;

	}else {
		_state = INS_INVALID;
	}

	return status;
}

void PumpSensor::stop(){
	if(_sensor.isOpen()){
		_sensor.stop();
	}

}

void PumpSensor::reset(){
 
}
 
bool PumpSensor::isConnected(){
	
	return _sensor.isOpen();
}
 
PumpHouseDevice::response_result_t
PumpSensor::rcvResponse(std::function<void(map<string,string>)> cb){

	PumpHouseDevice::response_result_t result = NOTHING;
	
	if(_state == INS_RESPONSE){
		result = PROCESS_VALUES;
		if(cb) (cb)(_resultMap);
		_state = INS_IDLE;
	}
	
	
done:
	
	if(result == CONTINUE)
		return result;

	if(result ==  INVALID){
		uint8_t sav =  LogMgr::shared()->_logFlags;
		START_VERBOSE;
		LogMgr::shared()->logTimedStampString("TankDepth INVALID: ");
		LogMgr::shared()->_logFlags = sav;
		return result;
	}
	
	return result;
}

 

PumpHouseDevice::device_state_t PumpSensor::getDeviceState(){
  
  device_state_t retval = DEVICE_STATE_UNKNOWN;
  
  if(!isConnected())
	  retval = DEVICE_STATE_DISCONNECTED;
  
  else if(_state == INS_INVALID)
	  retval = DEVICE_STATE_ERROR;
  
  else retval = DEVICE_STATE_CONNECTED;

  return retval;
}

void PumpSensor::idle(){
	
	if(isConnected() && (_state == INS_IDLE)){
		
		bool shouldQuery = false;
		
		if(_lastQueryTime.tv_sec == 0 &&  _lastQueryTime.tv_usec == 0 ){
			shouldQuery = true;
		} else {
			
			timeval now, diff;
			gettimeofday(&now, NULL);
			timersub(&now, &_lastQueryTime, &diff);
			
			if(diff.tv_sec >=  _queryDelay  ) {
				shouldQuery = true;
			}
		}
		
		
		if(shouldQuery){
		
	 
			uint8_t val = 0;
			
			if(_sensor.readInput(val)){
				
				// DEBUG - only two wires are hooked up now
  				val = val & 0x03;
			 
				//   0000 0  |	pressure Tank Runninng | Pump Running  | PumpPower

//				printf("TCA: %02x\n", val);
				_resultMap[_resultKey] =  std::bitset<8>(val).to_string();
				_state = INS_RESPONSE;
	 		}
			gettimeofday(&_lastQueryTime, NULL);
		}
	}
	

	}
