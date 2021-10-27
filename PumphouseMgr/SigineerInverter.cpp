//
//  SigineerInverter.cpp
//  pumphouse
//
//  Created by Vincent Moscaritolo on 9/7/21.
//

#include <bitset>
#include "SigineerInverter.hpp"
#include "LogMgr.hpp"
#include "Utils.hpp"
#include "PumphouseCommon.hpp"

// #define DEBUG_STREAM 1

SigineerInverter::SigineerInverter(PumpHouseDB *db){
	_db = db;
	_queryDelay 		= 5;	// seconds
	_timeoutDelay 	= 5;  //
	_maxRetryCount 	= 2;
	_retryCount  	= 0;
	_lastResponseTime = 0;
	
	_state 			= INS_UNKNOWN;
	_inv_state		= INV_UNKNOWN;
	reset();
}

SigineerInverter::~SigineerInverter(){
	stop();
}

void SigineerInverter::reset(){
	_isResponding 	= false;
	_isBypass 		= false;
	_isInverterOn	= false;
	_isFastCharge 	= false;
	_isFloatCharge 	= false;
	_response.clear();
	
	_lastQueryTime = {0,0};
	_resultMap.clear();
}


bool SigineerInverter::begin(string path, int *error){
	
	bool status = false;
	
	_db->getFloatProperty(string(PumpHouseDB::PROP_INVERTER_BATV_FLOAT),&_batVoltFloat);
	_db->getFloatProperty(string(PumpHouseDB::PROP_INVERTER_BATV_FAST),&_batVoltFast);
	
	status = _stream.begin(path.c_str(), B2400, error);
	_inv_state		= INV_UNKNOWN;
	_retryCount 		= 0;
	reset();
	
	if(status){
		_state 		= INS_IDLE;
	}
	else {
		_state = INS_INVALID;
		_db->logEvent(PumpHouseDB::EV_INV_FAIL);
	}
	
	return status;
}

void SigineerInverter::stop(){
	if(_stream.isOpen()){
		_stream.stop();
		_inv_state		= INV_UNKNOWN;
		_state 			= INS_UNKNOWN;
		reset();
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
	
	eventProcessor();
	
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
	
	else if(_inv_state == INV_NO_RESPONSE)
		retval = DEVICE_STATE_TIMEOUT;
	
	else retval = DEVICE_STATE_CONNECTED;
	
	return retval;
}


void SigineerInverter::idle() {
	
	if(isConnected()){
		
		switch (_state) {
			case INS_SENT_QUERY:
			case INS_RESPONSE:
				
				timeval now, diff;
				gettimeofday(&now, NULL);
				timersub(&now, &_lastQueryTime, &diff);
				
				if(diff.tv_sec >=  _timeoutDelay  ) {
					// we hit a timeout
					_retryCount++;
					_response.clear();
					_lastQueryTime = {0,0};
					_resultMap.clear();
					
					if(_retryCount < _maxRetryCount ){
						// retry again,.
						sendQuery();
					}
					else{
						// we exceeded the retry rate
						reset();		// reset inverter values
						_retryCount = 0;
						_state = INS_TIMEOUT;
						eventProcessor();
					}
					
				}
				
				break;
				
			case INS_IDLE:
			{
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
					sendQuery();
				}
			}
				
			default:;
		}
	}
}

void  SigineerInverter::sendQuery(){
	
#if DEBUG_STREAM
	printf("SEND Q1\n");
#endif
	write(getFD(), "Q1\r", 3);
	gettimeofday(&_lastQueryTime, NULL);
	_state = INS_SENT_QUERY;
}

//MARK: - input state machine


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
				// ignore INVERTER_BATTERY_TEMP - the inverter really doesnt read this
//				_resultMap[string(INVERTER_BATTERY_TEMP)] 	= v[6];
				_resultMap[string(INVERTER_UPS_STATUS)] 		= v[7];
				
				_lastResponseTime = time(NULL);
				
				_response.clear();
				_retryCount = 0;
				_isResponding = true;
				
				bitset<8> status = std::bitset<8>(v[7]);
				_isBypass = 			!status.test(7);
				_isInverterOn = 	status.test(7);
				
				float bat_volts = strtof(v[5].c_str(), NULL);
				_isFastCharge = (bat_volts >=  _batVoltFast);
				_isFloatCharge = (bat_volts >= _batVoltFloat) && !_isFastCharge ;
				eventProcessor()	;		// update state
				
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

//MARK: - event state machine

void  SigineerInverter::eventProcessor(){
	
	inverterState_t newState = _inv_state;
	
	if(_isResponding) {
		
		if(_isInverterOn){
			newState = INV_INVERTER;
		}
		else if(_isBypass) {
			newState = INV_BYPASS;
			
			if(_isFastCharge)
				newState = INV_FAST_CHARGE;
			
			else if(_isFloatCharge)
				newState = INV_FLOAT_CHARGE;
		}
	}
	else {
		// not responding
		if(_state == INS_TIMEOUT){
			newState = INV_NO_RESPONSE;
			// good place to restart the parsing
			_state = INS_IDLE;
		}
		
	}
	
	// Log change and set new state
	if(_inv_state != newState){
		switch (newState) {
			case  INV_NO_RESPONSE:
				_db->logEvent(PumpHouseDB::EV_INV_NOT_RESPONDING);
				LOGT_ERROR("Inverter Stopped Responding.\n");
				break;
				
			case  INV_BYPASS:
				_db->logEvent(PumpHouseDB::EV_INV_BYPASS);
				break;
				
			case  INV_INVERTER:
				_db->logEvent(PumpHouseDB::EV_INV_INVERTER);
				break;
				
			case  INV_FLOAT_CHARGE:
				_db->logEvent(PumpHouseDB::EV_INV_FLOAT);
				break;
				
			case  INV_FAST_CHARGE:
				_db->logEvent(PumpHouseDB::EV_INV_FAST_CHARGE);
				break;
				
				
			default:
				break;
		}
		_inv_state = newState;
	}
}
