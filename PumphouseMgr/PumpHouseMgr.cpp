//
//  PumpHouseMgr.cpp
//  wellpump
//
//  Created by Vincent Moscaritolo on 9/2/21.
//

#include "PumpHouseMgr.hpp"
#include "LogMgr.hpp"

const char* 	PumpHouseMgr::PumpHouseMgr_Version = "1.0.0 dev 1";

PumpHouseMgr::PumpHouseMgr(){
	// start the thread running
	_running = true;
	_state = STATE_INIT;
	_startTime = time(NULL);
	_thread = std::thread(&PumpHouseMgr::run, this);

}

PumpHouseMgr::~PumpHouseMgr(){
	_running = false;
	if (_thread.joinable())
		_thread.join();

}

long PumpHouseMgr::upTime(){
	time_t now = time(NULL);

	return now - _startTime;
}


string PumpHouseMgr::currentStateString(){
	
	string result;
	
	switch(_state){
			
		case STATE_INIT:
			result = "Initializing";
			break;
			
		case STATE_SETUP:
			result = "Schema Loaded";
			break;

		case STATE_READY:
			result = "Ready";
			break;
	
		default:
			result = "Unknown";
			break;
			
	}
	
	return result;
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
		_state = STATE_SETUP;
	}
 
	return success;
}


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


void PumpHouseMgr::stop(){
	
	LOGT_INFO("PumpHouseMgr STOP");
	stopInverter();
	stopShunt();
}

void PumpHouseMgr::stopInverter(){
	_inverter.stop();
}
										  
 void PumpHouseMgr::stopShunt(){
	 _smartshunt.stop();
 }
 
void PumpHouseMgr::run(){
	
	#define TIMEOUT_SEC 3		//Timeout parameter for select() - in seconds
	#define TIMEOUT_USEC 0		//Timeout parameter for select() - in micro seconds
	
	try{
		
		while(_running){
			
			if(_smartshunt.isConnected() || _inverter.isConnected() ) {
			
				_state = STATE_READY;
				
				int max_sd = 0;
				struct timeval timeout = {TIMEOUT_SEC, TIMEOUT_USEC};

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
		};
	}
	catch ( const SerialStreamException& e)  {
		printf("\tError %d %s\n\n", e.getErrorNumber(), e.what());
		
		if(e.getErrorNumber()	 == ENXIO){
	
		}
	}
}
