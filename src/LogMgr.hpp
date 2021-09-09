//
//  LogMgr.hpppp
//  plmtest
//
//  Created by Vincent Moscaritolo on 1/21/21.
//

#ifndef LogMgr_hpp
#define LogMgr_hpp

#include <stdio.h>
#include <stdarg.h>     /* va_list, va_start, va_arg, va_end */
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <string>
#include <mutex>
#include <stdio.h>
#include <iostream>
#include <fstream>


 
#define DPRINTF LogMgr::shared()->debugPrintf
#define TRACE_DATA_OUT(_buf_, _len_)  LogMgr::shared()->dumpTraceData(true,(const uint8_t* ) _buf_, (size_t)_len_)
#define TRACE_DATA_IN(_buf_, _len_)  LogMgr::shared()->dumpTraceData(false,(const uint8_t* ) _buf_, (size_t)_len_)

#define START_INFO {LogMgr::shared()->_logFlags |= LogMgr::LogLevelInfo;}
#define STOP_INFO {LogMgr::shared()->_logFlags  &= ~LogMgr::LogLevelInfo;}

#define START_DEBUG {LogMgr::shared()->_logFlags |= LogMgr::LogLevelDebug;}
#define STOP_DEBUG {LogMgr::shared()->_logFlags  &= ~LogMgr::LogLevelDebug;}

#define START_VERBOSE {LogMgr::shared()->_logFlags |= LogMgr::LogLevelVerbose;}
#define STOP_VERBOSE {LogMgr::shared()->_logFlags  &= ~LogMgr::LogLevelVerbose;}

 
using namespace std;

class LogMgr {

	static LogMgr *sharedInstance;

public:
 

	typedef enum  {
		/**
		 *  0...00001 LogFlagError
		 */
		LogFlagError      = (1 << 0),

		/**
		 *  0...00010 LogFlagWarning
		 */
		 LogFlagWarning    = (1 << 1),

		/**
		 *  0...00100 LogFlagInfo
		 */
		 LogFlagInfo       = (1 << 2),

		/**
		 *  0...01000 LogFlagDebug
		 */
		 LogFlagDebug      = (1 << 3),

		/**
		 *  0...10000 LogFlagVerbose
		 */
		 LogFlagVerbose    = (1 << 4)
	}logFLag_t;
 
	typedef enum {
		 /**
		  *  No logs
		  */
		 LogLevelOff       = 0,

		 /**
		  *  Error logs only
		  */
		 LogLevelError     = (LogMgr::LogFlagError),

		 /**
		  *  Error and warning logs
		  */
		 LogLevelWarning   = (LogLevelError   | LogMgr::LogFlagWarning),

		 /**
		  *  Error, warning and info logs
		  */
		 LogLevelInfo      = (LogLevelWarning | LogMgr::LogFlagInfo),

		 /**
		  *  Error, warning, info and debug logs
		  */
		 LogLevelDebug     = (LogLevelInfo    | LogMgr::LogFlagDebug),

		 /**
		  *  Error, warning, info, debug and verbose logs
		  */
		 LogLevelVerbose   = (LogLevelDebug   | LogMgr::LogFlagVerbose),

		 /**
		  *  All logs (1...11111)
		  */
		 LogLevelAll       = LogLevelVerbose
	} logLevel_t;
	
	
	static LogMgr *shared() {
			if (!sharedInstance)
				sharedInstance = new LogMgr;
			return sharedInstance;
		}

	LogMgr();
	
	bool setLogFilePath(string path );
 
	void logMessage(logFLag_t level, bool logTime, string str);
	void logMessage(logFLag_t level, bool logTime, const char *fmt, ...);
	void logTimedStampString(const string  str);

	void dumpTraceData(bool outgoing, const uint8_t* data, size_t len);
	void writeToLog(const string str);
  
	bool _logBufferData;
	
	uint8_t _logFlags;


private:
	
	void writeToLog(const uint8_t*, size_t);

	mutable std::mutex _mutex;
	std::ofstream		_ofs;

};


#define LOG_MESSAGE(_level_, _msg_, ...)  LogMgr::shared()->logMessage(_level_, false, _msg_, ##__VA_ARGS__)
#define LOG_DEBUG( _msg_, ...)  LogMgr::shared()->logMessage(LogMgr::LogFlagDebug,  false, _msg_, ##__VA_ARGS__)
#define LOG_INFO( _msg_, ...)  LogMgr::shared()->logMessage(LogMgr::LogFlagInfo,  false, _msg_, ##__VA_ARGS__)
#define LOG_WARNING( _msg_, ...)  LogMgr::shared()->logMessage(LogMgr::LogFlagWarning,  false, _msg_, ##__VA_ARGS__)
#define LOG_ERROR( _msg_, ...)  LogMgr::shared()->logMessage(LogMgr::LogFlagError,  false, _msg_, ##__VA_ARGS__)

#define LOGT_MESSAGE(_level_, _msg_, ...)  LogMgr::shared()->logMessage(_level_,  true,_msg_, ##__VA_ARGS__)
#define LOGT_DEBUG( _msg_, ...)  LogMgr::shared()->logMessage(LogMgr::LogFlagDebug, true,_msg_, ##__VA_ARGS__)
#define LOGT_INFO( _msg_, ...)  LogMgr::shared()->logMessage(LogMgr::LogFlagInfo, true,_msg_, ##__VA_ARGS__)
#define LOGT_WARNING( _msg_, ...)  LogMgr::shared()->logMessage(LogMgr::LogFlagWarning,true, _msg_, ##__VA_ARGS__)
#define LOGT_ERROR( _msg_, ...)  LogMgr::shared()->logMessage(LogMgr::LogFlagError, true, _msg_, ##__VA_ARGS__)

 
#endif /* LogMgr_hpp */
