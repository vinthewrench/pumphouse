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
	_state = INS_UNKNOWN;
	_values.clear();
}

SmartShunt::~SmartShunt(){
 
}


bool SmartShunt::begin(const char * path, int *error){
	bool status = false;
	
	status = _stream.begin(path, B19200, error);
	if(status){
		_state = INS_IDLE;
		
		_checkSum = 0;
		_rName.clear();
		_rValue.clear();
		_resultMap.clear();
		_values.clear();

	}else {
		_state = INS_UNKNOWN;
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

 
SmartShunt::smartshunt_result_t
		SmartShunt::rcvResponse(std::function<void(map<string,string>)> cb){
 
	smartshunt_result_t result = NOTHING;

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


void SmartShunt::idle() {
	
}

//MARK: - state machine


SmartShunt::smartshunt_result_t SmartShunt::process_char( uint8_t ch){

	smartshunt_result_t retval = CONTINUE;

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
				retval = PROCESS_VALUES;
			}
		
			_state = INS_IDLE;
			_checkSum = 0;
			break;
 
		default:
 
			break;
	}
	
	return retval;
}

