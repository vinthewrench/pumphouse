//
//  LogMgr.cpp
//  plmtest
//
//  Created by Vincent Moscaritolo on 1/21/21.
//

#include "LogMgr.hpp"

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>

#include "TimeStamp.hpp"

using namespace std;
using namespace timestamp;


LogMgr *LogMgr::sharedInstance = 0;

LogMgr::LogMgr() {
	_logBufferData = false;
	_logFlags = 0;
}
 
void LogMgr::logMessage(logFLag_t level, bool logTime, std::string str){
	if((_logFlags & level) == 0) return;
	if(logTime)
		logTimedStampString(str);
	else
		writeToLog(str);
}


void LogMgr::logTimedStampString(const string  str){
	writeToLog( TimeStamp(false).logFileString() + "\t" + str + "\n");
}

void LogMgr::logMessage(logFLag_t level,bool logTime,  const char *fmt, ...){

	if((_logFlags & level) == 0) return;
 
	char buff[1024];
	
	va_list args;
	va_start(args, fmt);
	vsprintf(buff, fmt, args);
 	va_end(args);
	
	if(logTime)
		logTimedStampString(string(buff));
	else
		writeToLog(string(buff));
 }

void LogMgr::dumpTraceData(bool outgoing, const uint8_t* data, size_t len){

	if((_logFlags & LogFlagVerbose) == 0) return;

	char hexDigit[] = "0123456789ABCDEF";

	char 	lineBuf[1024];
	char 	*p = lineBuf;
	
	if(outgoing) {
		p += sprintf(p, "\t (%02zu) -->  ", len);
	}else {
		p += sprintf(p, "\t (%02zu) <--  02 ", len +1);
		
		if(len == 0){
			;
		}
 	}
	
	for(size_t i = 0; i<len; i++) {
		*p++ = hexDigit[ data[i] >>4];
		*p++ = hexDigit[ data[i] &0xF];
		*p++ = ' ';
	}
	*p++ = '\n';

	this->writeToLog((const uint8_t*)lineBuf, (size_t) (p-lineBuf));
	
}

void LogMgr::writeToLog(const std::string str){
	
	writeToLog((const uint8_t*)str.c_str(), str.length());
}

void LogMgr::writeToLog(const uint8_t* buf, size_t len){
	
	lock_guard<std::mutex> lock(_mutex);

	if(len){
 		printf("%.*s",(int)len, buf);
		if(_ofs.is_open()) {
			_ofs.write( (char*)buf, len);
			_ofs.flush();
		}
	}
}


bool LogMgr::setLogFilePath(string path ){
	bool  statusOk = false;
	
	if( path.size() == 0 ){
		if(_ofs.is_open()) {
			_ofs.flush();
			_ofs.close();
			statusOk = true;
		}
	}
	else {
		
		try{
			
			if(_ofs.is_open()) {
				_ofs.flush();
				_ofs.close();
			}
			
			_ofs.open(path, std::ios_base::app);
		
			if(_ofs.fail())
				return false;
			
			logTimedStampString("Log Start");
			statusOk = true;
		}
		catch(std::ofstream::failure &writeErr) {
			statusOk = false;
		}
		
	}
	return statusOk;
}
