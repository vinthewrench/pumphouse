//
//  SerialStream.cpp
//  wellpump
//
//  Created by Vincent Moscaritolo on 9/2/21.
//

#include "SerialStream.hpp"
#include <errno.h>
#include <sys/ioctl.h>                                                  // Serial Port IO Controls
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include "LogMgr.hpp"


SerialStream::SerialStream(){
	_isSetup = false;
	_fd = -1;
}

SerialStream::~SerialStream(){
	stop();
	
}

bool SerialStream::begin(const char * path){
	int error = 0;
	
	return begin(path, error);
}



bool SerialStream::begin(const char * path, speed_t speed,  int * errorOut){
	
	struct termios tty;
	bzero (&tty,  sizeof tty);

	_isSetup = false;

	_fd = open( path, O_RDWR| O_NOCTTY  | O_NONBLOCK | O_NDELAY );
	
	if(_fd == -1){
		if(errorOut) *errorOut = errno;
		return false;
	}
	
	/* Error Handling */
	if ( tcgetattr ( _fd, &tty ) != 0 ) {
		return false;
	}
	
	/* Save old tty parameters */
	_savetty = tty;

	/* Set Baud Rate */
	cfsetospeed (&tty, (speed_t)speed); //	Set write  Speed
	cfsetispeed (&tty, (speed_t)speed); //  Set Read  Speed

	/* Setting other Port Stuff */
	tty.c_cflag     &=  ~(tcflag_t)PARENB;            // Make 8n1
	tty.c_cflag     &=  ~(tcflag_t)CSTOPB;
	tty.c_cflag     &=  ~(tcflag_t)CSIZE;
	tty.c_cflag     |=  CS8;
	
	tty.c_cflag     &=  ~(tcflag_t)CRTSCTS;           // no flow control
	tty.c_cc[VMIN]   =  1;                  // read doesn't block
	tty.c_cc[VTIME]  =  5;                  // 0.5 seconds read timeout
	tty.c_cflag     |=  CREAD | CLOCAL;     // turn on READ & ignore ctrl lines

	/* Make raw */
	cfmakeraw(&tty);
	
	/* Flush Port, then applies attributes */
	tcflush( _fd, TCIFLUSH );
	
	if ( tcsetattr ( _fd, TCSANOW, &tty ) != 0) {
		return false;
	}

//	// flush anything else left
//	while(true){
//		u_int8_t c;
//		size_t n =  (size_t)::read( _fd, &c, 1 );
//		if(n == 1){
//			printf("%02X ", c);
//		}
//		if(n == 0) break;
// 		if(n == SIZE_MAX) break;
//	}

	_hasChar = false;
	_isSetup = true;
	
	return true;
}

bool SerialStream::isOpen(){
	return _isSetup;
	
};
	



void SerialStream::stop(){

	if(_isSetup){
		tcflush(_fd,TCIOFLUSH);
		tcsendbreak(_fd, 100);
		tcsetattr ( _fd, TCSANOW, &_savetty );
		close(_fd);
	}
	
	_isSetup = false;

}

ssize_t SerialStream::write(const uint8_t* buf, size_t len){
	
	if(!_isSetup) return EBADF;
 
 	TRACE_DATA_OUT( buf,len);
	
	return ::write(_fd, buf, len);
 }


int SerialStream::available(){
	
	if(!_isSetup) return 0;
 
	if(_hasChar) return 1;

	u_int8_t c;
	size_t n =  (size_t)::read( _fd, &c, 1 );

	if(n == -1){
		int lastError = errno;
		
		// no chars waiting
		if(lastError == EAGAIN)
			return 0;
		
		// device is broken
		if(lastError == ENXIO ) {
			
			throw SerialStreamException(std::string(strerror(lastError)), lastError);

//			_isSetup = false;
//
//			return -1;
		}
		
	}else if(n == 1) {
		_hasChar = true;
		_peek_c = c;
		return 1;
	}
	
	return 0;
}


 int SerialStream::read(){

	 if(available()){
		 _hasChar = false;
		 return _peek_c;
	 }
	 
	return -1;
}


void SerialStream::printf(const char *fmt, ...){
	
	if(!_isSetup) return;
	
	char buff[1024];
	
	va_list args;
	va_start(args, fmt);
	size_t cnt = (size_t)vsprintf(buff, fmt, args);
	
	this->write((const uint8_t*)buff,cnt);
	
	va_end(args);
}
