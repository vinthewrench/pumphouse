//
//  SigineerInverter.cpp
//  pumphouse
//
//  Created by Vincent Moscaritolo on 9/7/21.
//

#include "SigineerInverter.hpp"
#include "LogMgr.hpp"

#include "Utils.hpp"

SigineerInverter::SigineerInverter(){
	_state = INS_UNKNOWN;
	_lastQueryTime = {0,0};
	_resultMap.clear();
}

SigineerInverter::~SigineerInverter(){
	stop();
}


bool SigineerInverter::begin(string path, int *error){
	
	bool status = false;
	
	status = _stream.begin(path.c_str(), B2400, error);
 
	if(status){
		_state = INS_IDLE;
		_response.clear();
		_queryDelay = 10;	// seconds
		_lastQueryTime = {0,0};
		_resultMap.clear();
		
	}else {
		_state = INS_INVALID;
	}
	
	return status;
}

void SigineerInverter::stop(){
	if(_stream.isOpen()){
		_stream.stop();
	}
}

 

bool SigineerInverter::isConnected(){
	return _stream.isOpen();
}
 
PumpHouseDevice::response_result_t
			SigineerInverter::rcvResponse(std::function<void(map<string,string>)> cb){

	PumpHouseDevice::response_result_t result = NOTHING;

	if(!_stream.isOpen()) {
		return ERROR;
	}
 
	while(_stream.available()) {
 
		uint8_t ch = _stream.read();
		result = process_char(ch);
		
		if(result == PROCESS_VALUES){
			if(cb) (cb)(_resultMap);
		}
	}
	
done:
	
	if(result == CONTINUE)
		return result;

	if(result ==  INVALID){
		uint8_t sav =  LogMgr::shared()->_logFlags;
		START_VERBOSE;
		LogMgr::shared()->logTimedStampString("SmartShunt INVALID: ");
		LogMgr::shared()->_logFlags = sav;
		return result;
	}
 
	return result;
}

 
PumpHouseDevice::device_state_t SigineerInverter::getDeviceState(){
	
	device_state_t retval = DEVICE_STATE_UNKNOWN;
	
	if(!isConnected())
		retval = DEVICE_STATE_DISCONNECTED;
	
	else if(_state == INS_INVALID)
		retval = DEVICE_STATE_ERROR;
	
	else retval = DEVICE_STATE_CONNECTED;
 
	return retval;
}


void SigineerInverter::idle() {
	
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
			write(getFD(), "Q1\r", 3);
			_state = INS_SENT_QUERY;
			gettimeofday(&_lastQueryTime, NULL);
			}
	}
}


//MARK: - state machine


PumpHouseDevice::response_result_t SigineerInverter::process_char( uint8_t ch){
	
	PumpHouseDevice::response_result_t retval = CONTINUE;

#if DEBUG_STREAM

	if(iscntrl(ch)){
		printf( "%02x\n", ch);
	}else
	{
		printf( "%02x |%c|\n", ch, ch);
	}

#endif

	switch (_state) {

		case INS_SENT_QUERY:
			if(ch == '('){
				_response.clear();
				_state = INS_RESPONSE;
			}
			break;
	
		case INS_RESPONSE:
			if(ch == 0x0D){
				
				vector<string> v = split<string>(_response, " ");
	 	 		// process response;

				_resultMap[string(INVERTER_IP_VOLTS)] 			= v[0];
				_resultMap[string(INVERTER_IP_FAULT_VOLTS)]  = v[1];
				_resultMap[string(INVERTER_OP_VOLTS)] 			= v[2];
				_resultMap[string(INVERTER_OP_CURRENT)] 		= v[3];
				_resultMap[string(INVERTER_OP_FREQ)] 			= v[4];
				_resultMap[string(INVERTER_BATTERY_V)] 		= v[5];
				_resultMap[string(INVERTER_BATTERY_TEMP)] 	= v[6];
				_resultMap[string(INVERTER_UPS_STATUS)] 		= v[7];
 
				// we have a set of good values
				retval = PROCESS_VALUES;
	
				_state = INS_IDLE;
			}
			else{
				_response.push_back(ch);
			}
			
		default:
			break;
	}
	
	return retval;
 }
