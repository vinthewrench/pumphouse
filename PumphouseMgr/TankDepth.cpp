//
//  TankDepth.cpp
//  pumphouse
//
//  Created by Vincent Moscaritolo on 9/12/21.
//

#include "TankDepth.hpp"
#include "LogMgr.hpp"

TankDepth::TankDepth( PumpHouseDB *db){
	_db = db;
	_state = INS_UNKNOWN;
	_lastQueryTime = {0,0};
	_resultMap.clear();
	
	// default numbers
	_valFull =  15000;
	_valEmpty =  7667;
 
}

TankDepth::~TankDepth(){
	stop();
}


 
bool TankDepth::begin(int deviceAddress, string resultKey, string rawResultKey,  int *error){
	bool status = false;

	
	status = _sensor.begin(deviceAddress, error);
	
	if(status){
		_state = INS_IDLE;
		_queryDelay = 5;	// seconds
		_lastQueryTime = {0,0};
		_resultMap.clear();
		_resultKey = resultKey;
		_rawResultKey = rawResultKey;

	}else {
		_state = INS_INVALID;
	}
	

	return status;
}

void TankDepth::stop(){
	if(_sensor.isOpen()){
		_sensor.stop();
	}

}

bool TankDepth::isConnected(){
	return _sensor.isOpen();
}
 
PumpHouseDevice::response_result_t
TankDepth::rcvResponse(std::function<void(map<string,string>)> cb){

	PumpHouseDevice::response_result_t result = NOTHING;
	
	if(!_sensor.isOpen()) {
		return ERROR;
	}
	
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

 

PumpHouseDevice::device_state_t TankDepth::getDeviceState(){
  
  device_state_t retval = DEVICE_STATE_UNKNOWN;
  
  if(!isConnected())
	  retval = DEVICE_STATE_DISCONNECTED;
  
  else if(_state == INS_INVALID)
	  retval = DEVICE_STATE_ERROR;
  
  else retval = DEVICE_STATE_CONNECTED;

  return retval;
}

void TankDepth::idle(){
	
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
			
			uint16_t rawData = 0;
	
			_db->getUint16Property(string(PumpHouseDB::PROP_TANK_FULL),&_valFull);
			_db->getUint16Property(string(PumpHouseDB::PROP_TANK_EMPTY),&_valEmpty);
			
			auto gain = MCP3427::GAIN_1X;
			auto adcBits = MCP3427::ADC_16_BITS;
			
			if(_sensor.analogRead(	rawData, 0, gain, adcBits))
			{
				if(!_rawResultKey.empty()){
					_resultMap[_rawResultKey] =  to_string(rawData);
				}
				
				// max out result
				if(rawData < _valEmpty) rawData = _valEmpty;
				else if (rawData > _valFull) rawData = _valFull;
				float depth = (float((rawData - _valEmpty)) /float(( _valFull - _valEmpty))) * 100.0;
 			
	 			_resultMap[_resultKey] =  to_string(depth);
				_state = INS_RESPONSE;
				gettimeofday(&_lastQueryTime, NULL);
				
			}
			else
			{
//				_state = INS_INVALID;
			}
			
		}
	}
}
