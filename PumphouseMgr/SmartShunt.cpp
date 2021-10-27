//
//  SmartShunt.cpp
//  pumphouse
//
//  Created by Vincent Moscaritolo on 9/7/21.
//

#include "SmartShunt.hpp"
#include "LogMgr.hpp"

#define DEBUG_STREAM 0


SmartShunt::SmartShunt(){
	_state = INS_INVALID;
	_values.clear();
	_lastResponseTime = 0;

}

SmartShunt::~SmartShunt(){
 
}


bool SmartShunt::begin(string path, int *error){
	bool status = false;
	
	status = _stream.begin(path.c_str(), B19200, error);
	if(status){
		_state = INS_STARTED;
		_checkSum = 0;
		_rName.clear();
		_rValue.clear();
		_resultMap.clear();
		_values.clear();

	}else {
		_state = INS_INVALID;
	}
	
	return status;

}

void SmartShunt::stop(){
	
	if(_stream.isOpen()){
		_stream.stop();
	}
}
 
bool SmartShunt::isConnected(){
	return _stream.isOpen();
}

void SmartShunt::reset(){
	_state = INS_IDLE;
	
	_checkSum = 0;
	_rName.clear();
	_rValue.clear();
	_resultMap.clear();
	_values.clear();

}

 
PumpHouseDevice::response_result_t
SmartShunt::rcvResponse(std::function<void(map<string,string>)> cb){
	
	PumpHouseDevice::response_result_t result = PumpHouseDevice::NOTHING;
	
	if(!_stream.isOpen()) {
		return PumpHouseDevice::ERROR;
	}
	
	if(_state == INS_STARTED){
		_state = INS_IDLE;
		_resultMap.clear();
		// nothing  to do here..
		//				_resultMap[string(INVERTER_COMMUNICATIONS)]  =  to_string(RESTARTED);
		if(cb) (cb)(_resultMap);
		return PumpHouseDevice::PROCESS_VALUES;
	}
	
	while(_stream.available()) {
		uint8_t ch = _stream.read();
		result = process_char(ch);
		
		if(result == PumpHouseDevice::PROCESS_VALUES){
			if(cb) (cb)(_resultMap);
		}
	}
	
done:
	
	if(result == PumpHouseDevice::CONTINUE)
		return result;
	
	if(result ==  PumpHouseDevice::INVALID){
		uint8_t sav =  LogMgr::shared()->_logFlags;
		START_VERBOSE;
		LogMgr::shared()->logTimedStampString("SmartShunt INVALID: ");
		LogMgr::shared()->_logFlags = sav;
		return result;
	}
	
	return result;
}


void SmartShunt::idle() {
	
}

PumpHouseDevice::device_state_t SmartShunt::getDeviceState(){
	
 	device_state_t retval = DEVICE_STATE_UNKNOWN;
	
	if(!isConnected())
		retval = DEVICE_STATE_DISCONNECTED;
	
	else if(_state == INS_INVALID)
		retval = DEVICE_STATE_ERROR;
	
	else retval = DEVICE_STATE_CONNECTED;
 
	return retval;
}


//MARK: - state machine


PumpHouseDevice::response_result_t SmartShunt::process_char( uint8_t ch){

	PumpHouseDevice::response_result_t retval = PumpHouseDevice::CONTINUE;

#if DEBUG_STREAM
 
		if(iscntrl(ch)){
			printf( "%02x\n", ch);
		}else
		{
			printf( "%02x |%c|\n", ch, ch);
		}
#endif
	
	switch (_state) {

		case INS_IDLE:
	
			if(ch == 0x0D){
				_rName.clear();
				_rValue.clear();
				_resultMap.clear();
				_checkSum += ch;
 				_state = INS_RECORD_BEGIN;
 			}
			break;
			
		case INS_RECORD_BEGIN:
			_checkSum += ch;
			if(ch == 0x0A){
				/* ignore */
			}
			else {
				_rName.push_back(ch);
				_state = INS_RECORD_NAME;
			}
	 		break;
			
		case INS_RECORD_NAME:
			
			_checkSum += ch;
			if(ch == 0x09){
				if(_rName == "Checksum"){
					// process checksum
					_state = INS_CHECKSUM;
				}
				else{
					_state = INS_RECORD_VALUE;
				}
			}else {
				_rName.push_back(ch);
			}
			break;
			
		case INS_RECORD_VALUE:
			_checkSum += ch;

			if(ch == 0x0d){
				// save  this
				_resultMap[_rName] = _rValue;
				
				_rName.clear();
				_rValue.clear();
				_state = INS_RECORD_BEGIN;
 			}
			else {
				_rValue.push_back(ch);
			}
			break;
			
		case INS_CHECKSUM:
			_checkSum += ch;

			if(_checkSum == 0){
				// we have a set of good values
				retval = PumpHouseDevice::PROCESS_VALUES;
				_lastResponseTime = time(NULL);
	
			}
		
			_state = INS_IDLE;
			_checkSum = 0;
			break;
 
		default:
 
			break;
	}
	
	return retval;
}

